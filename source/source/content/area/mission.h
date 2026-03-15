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
    MISSION_PRESET_GROW_MANY_PIKMIN,
    
    //Collect as many treasures as you can within the time limit.
    //Medal depends on how many got collected, platinum for all. Matches P3.
    MISSION_PRESET_COLLECT_TREASURE,
    
    //Defeat as many enemies as you can within the time limit.
    //Medal depends on how many were defeated, platinum for all. Matches P3.
    MISSION_PRESET_BATTLE_ENEMIES,
    
    //Defeat the boss within the time limit.
    //Medal depends on time taken. Matches P3.
    MISSION_PRESET_DEFEAT_BOSSES,
    
    //Collect as many treasures and enemies as you can within the time limit.
    //Medal depends on amount collected. Matches P4.
    MISSION_PRESET_COLLECT_EVERYTHING,
    
    //Custom rules.
    MISSION_PRESET_CUSTOM,
    
};


//Mission preset enum naming.
buildEnumNames(missionPresetNames, MISSION_PRESET)({
    { MISSION_PRESET_GROW_MANY_PIKMIN, "Grow Many Pikmin" },
    { MISSION_PRESET_COLLECT_TREASURE, "Collect Treasure" },
    { MISSION_PRESET_BATTLE_ENEMIES, "Battle Enemies" },
    { MISSION_PRESET_DEFEAT_BOSSES, "Defeat Bosses" },
    { MISSION_PRESET_COLLECT_EVERYTHING, "Collect Everything" },
    { MISSION_PRESET_CUSTOM, "Custom" },
});


//Possible non-script end conditions for missions.
enum MISSION_END_COND {

    //Ended through the pause menu.
    MISSION_END_COND_PAUSE_MENU,
    
    //Specified amount of target mobs were cleared.
    MISSION_END_COND_MOB_GROUP,
    
    //Time limit was reached.
    MISSION_END_COND_TIME_LIMIT,
    
    //Specified amount of leaders entered the specified in-area region.
    MISSION_END_COND_LEADERS_IN_REGION,
    
    //Pikmin count reached specified amount or more.
    MISSION_END_COND_PIKMIN_OR_MORE,
    
    //Pikmin count reached specified amount or fewer.
    MISSION_END_COND_PIKMIN_OR_FEWER,
    
    //Specified amount of Pikmin was lost.
    MISSION_END_COND_LOSE_PIKMIN,
    
    //Specified amount of leaders were lost.
    MISSION_END_COND_LOSE_LEADERS,
    
    //A leader took damage.
    MISSION_END_COND_TAKE_DAMAGE,
    
    //Reserved for script use.
    MISSION_END_COND_SCRIPT,
    
};


//Types of mission mob groups.
enum MISSION_MOB_GROUP {

    //Mobs from the given list.
    MISSION_MOB_GROUP_CUSTOM,
    
    //Treasures and treasure-like objects.
    MISSION_MOB_GROUP_TREASURES,
    
    //Enemies and enemy-like objects.
    MISSION_MOB_GROUP_ENEMIES,
    
    //Treasures, treasure-like objects, enemies, and enemy-like objects.
    MISSION_MOB_GROUP_TREASURES_ENEMIES,
    
    //Leader objects.
    MISSION_MOB_GROUP_LEADERS,
    
    //Pikmin objects.
    MISSION_MOB_GROUP_PIKMIN,
    
};


//Mission mob GROUP type enum naming.
buildEnumNames(missionMobGroupTypeNames, MISSION_MOB_GROUP)({
    { MISSION_MOB_GROUP_CUSTOM, "Custom" },
    { MISSION_MOB_GROUP_TREASURES, "Treasures" },
    { MISSION_MOB_GROUP_ENEMIES, "Enemies" },
    { MISSION_MOB_GROUP_TREASURES_ENEMIES, "Treasures and enemies" },
    { MISSION_MOB_GROUP_LEADERS, "Leaders" },
    { MISSION_MOB_GROUP_PIKMIN, "Pikmin" },
});


//Scoring criterion types for score-based missions.
enum MISSION_SCORE_CRITERION {

    //Amount of mobs from a group cleared.
    MISSION_SCORE_CRITERION_MOB_GROUP,
    
    //Total amount of living Pikmin.
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
    { MISSION_SCORE_CRITERION_MOB_GROUP, "Mob group target" },
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
    
    //Health bar.
    MISSION_HUD_ITEM_CONTENT_HEALTH,
    
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
    { MISSION_HUD_ITEM_CONTENT_HEALTH, "Health bar" },
    { MISSION_HUD_ITEM_CONTENT_CUR_TOT, "Current amount / total" },
    { MISSION_HUD_ITEM_CONTENT_REM_TOT, "Remaining amount / total" },
    { MISSION_HUD_ITEM_CONTENT_CUR_AMT, "Current amount" },
    { MISSION_HUD_ITEM_CONTENT_REM_AMT, "Remaining amount" },
    { MISSION_HUD_ITEM_CONTENT_TOT_AMT, "Total amount" },
});


//Types of things a mission HUD item can show amounts of.
enum MISSION_HUD_ITEM_AMT {

    //Amounts in one or more mob groups.
    MISSION_HUD_ITEM_AMT_MOB_GROUP,
    
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
    { MISSION_HUD_ITEM_AMT_MOB_GROUP, "Mobs from groups" },
    { MISSION_HUD_ITEM_AMT_LEADERS_IN_REGION, "Leaders in regions" },
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


//Possible ways of awarding the player a medal for a mission.
enum MISSION_MEDAL_AWARD_MODE {

    //Based on points in different criteria.
    MISSION_MEDAL_AWARD_MODE_POINTS,
    
    //Based on whether the player cleared the mission or not.
    MISSION_MEDAL_AWARD_MODE_CLEAR,
    
    //Based on whether the player played or not.
    MISSION_MEDAL_AWARD_MODE_PARTICIPATION,
    
};


//DEPRECATED by MISSION_END_COND in 1.2.0.
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


//Mission goal enum naming.
buildEnumNames(missionGoalNames, MISSION_GOAL)({
    { MISSION_GOAL_END_MANUALLY, "End whenever you want" },
    { MISSION_GOAL_COLLECT_TREASURE, "Collect treasures" },
    { MISSION_GOAL_BATTLE_ENEMIES, "Battle enemies" },
    { MISSION_GOAL_TIMED_SURVIVAL, "Survive" },
    { MISSION_GOAL_GET_TO_EXIT, "Get to the exit" },
    { MISSION_GOAL_GROW_PIKMIN, "Grow Pikmin" },
});


//DEPRECATED by MISSION_END_COND in 1.2.0.
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
 * @brief Represents a non-script end condition in the mission.
 */
struct MissionEndCond {

    //--- Public members ---
    
    //Type of condition.
    MISSION_END_COND type = MISSION_END_COND_PAUSE_MENU;
    
    //Index-related parameter, if applicable. Can be used for other things too.
    size_t indexParam = 0;
    
    //Amount-related parameter, if applicable. Can be used for other things too.
    size_t amountParam = 1;
    
    //Whether a medal can be obtained when the mission ends.
    bool clear = false;
    
    //Whether the time remaining becomes 0 for scoring purposes, if applicable.
    bool zeroTimeForScore = false;
    
    //Whether to use a neutral "Mission over!" big message and a neutral
    //jingle, or to use a message and jingle that depend on clear/failure.
    bool neutralMood = false;
    
    //Text explaining the reason behind the mission ending,
    //to show in the results menu.
    string reason;
    
};


/**
 * @brief A group of mobs that is relevant to a mission.
 */
struct MissionMobGroup {

    //--- Public members ---
    
    //Type.
    MISSION_MOB_GROUP type = MISSION_MOB_GROUP_CUSTOM;
    
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
    MISSION_SCORE_CRITERION type = MISSION_SCORE_CRITERION_MOB_GROUP;
    
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
    
    //List of mob groups or regions to account for, if applicable.
    vector<size_t> idxsList;
    
};


/**
 * @brief Info about a given area's mission.
 */
struct MissionData {

    //--- Public members ---
    
    //Preset. Only really used for the editor's GUI.
    MISSION_PRESET preset = MISSION_PRESET_CUSTOM;
    
    //Mission end conditions.
    vector<MissionEndCond> endConds;
    
    //Mob groups.
    vector<MissionMobGroup> mobGroups;
    
    //Mission medal award mode.
    MISSION_MEDAL_AWARD_MODE medalAwardMode = MISSION_MEDAL_AWARD_MODE_CLEAR;
    
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
    
    //Text briefing the player on the mission's objectives.
    string briefingObjective;
    
    //Text bullet points briefing the player on the mission's misc. notes.
    vector<string> briefingNotes;
    
    //The maker's record.
    int makerRecord = 0;
    
    //The date of the maker's record, or empty for no record.
    string makerRecordDate = "";
    
    
    //--- Public function declarations ---
    
    void applyPreset(MISSION_PRESET newPreset);
    string getBriefingObjectiveText() const;
    vector<string> getNoteBulletPoints() const;
    vector<string> getMedalAwardBulletPoints() const;
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
    
    //Mission medal award mode.
    MISSION_MEDAL_AWARD_MODE medalAwardMode = MISSION_MEDAL_AWARD_MODE_CLEAR;
    
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
    
    //Score obtained.
    int score = 0;
    
    //Date of the record.
    string date;
    
    
    //--- Public function declarations ---
    
    void clear();
    bool loadFromDataNode(DataNode* node);
    bool saveToDataNode(DataNode* node);
    bool isPlatinum(const MissionData& mission);
    
};


#pragma endregion
#pragma region End condition types


/**
 * @brief Class interface for a mission end condition type.
 */
class MissionEndCondType {

public:

    //--- Public misc. definitions ---
    
    /**
     * @brief Information that's only useful for the editor.
     */
    struct EditorInfo {
    
        //--- Public members ---
        
        //Description of the condition.
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
        
        //A description of the condition. Empty if not used.
        string description;
        
    };
    
    
    //--- Public function declarations ---
    
    virtual ~MissionEndCondType() = default;
    virtual string getName() const = 0;
    virtual EditorInfo getEditorInfo() const = 0;
    virtual HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const = 0;
    virtual bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const = 0;
    virtual bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const = 0;
    
};


/**
 * @brief Class representing the "pause menu" mission end condition.
 */
class MissionEndCondTypePauseMenu : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "mob group" mission end condition.
 */
class MissionEndCondTypeMobGroup : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "time limit" mission end condition.
 */
class MissionEndCondTypeTimeLimit : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "leaders in region" mission end condition.
 */
class MissionEndCondTypeLeadersInRegion : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin or more" mission end condition.
 */
class MissionEndCondTypePikminOrMore : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin or fewer" mission end condition
 */
class MissionEndCondTypePikminOrFewer : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "lose Pikmin" mission end condition.
 */
class MissionEndCondTypeLosePikmin : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "lose leaders" mission end condition.
 */
class MissionEndCondTypeLoseLeaders : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "take damage" mission end condition.
 */
class MissionEndCondTypeTakeDamage : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


/**
 * @brief Class representing the "script" mission end condition.
 */
class MissionEndCondTypeScript : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    EditorInfo getEditorInfo() const override;
    HudInfo getHudInfo(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    bool getZoomData(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay,
        Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(
        MissionEndCond* cond, MissionData* mission, GameplayState* gameplay
    ) const override;
    
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
    virtual string getFriendlyName() const = 0;
    virtual size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const = 0;
    
};


/**
 * @brief Class representing the "mob group mob" mission score criterion.
 */
class MissionScoreCriterionTypeMobGroup : public MissionScoreCriterionType {

public:

    //--- Public function declarations ---
    
    string getName() const override;
    string getFriendlyName() const override;
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
    string getFriendlyName() const override;
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
    string getFriendlyName() const override;
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
    string getFriendlyName() const override;
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
    string getFriendlyName() const override;
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
    string getFriendlyName() const override;
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
    string getFriendlyName() const override;
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
    string getFriendlyName() const override;
    size_t calculateAmount(
        MissionScoreCriterion* cri,
        MissionData* mission, GameplayState* gameplay
    ) const override;
    
};


#pragma endregion
