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

    //Collect as many Pikmin as you can within the time limit.
    //Medal depends on how many you grew. Matches P1.
    MISSION_PRESET_COLLECT_PIKMIN,
    
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
    { MISSION_PRESET_COLLECT_PIKMIN, "Collect Pikmin" },
    { MISSION_PRESET_COLLECT_TREASURE, "Collect Treasure" },
    { MISSION_PRESET_BATTLE_ENEMIES, "Battle Enemies" },
    { MISSION_PRESET_DEFEAT_BOSSES, "Defeat Bosses" },
    { MISSION_PRESET_COLLECT_EVERYTHING, "Collect Everything" },
    { MISSION_PRESET_CUSTOM, "Custom" },
});


//Something that can be measured in the mission.
enum MISSION_METRIC {

    //Number of mobs cleared from a given mob group.
    MISSION_METRIC_MOB_GROUP_CLEARED_MOBS,
    
    //Combined health of mobs in a given mob group.
    MISSION_METRIC_MOB_GROUP_HEALTH,
    
    //Seconds left in the time limit.
    MISSION_METRIC_SECS_LEFT,
    
    //Seconds passed.
    MISSION_METRIC_SECS_PASSED,
    
    //How many leaders are in a given region.
    MISSION_METRIC_LEADERS_IN_REGION,
    
    //Total number of living Pikmin.
    MISSION_METRIC_LIVING_PIKMIN,
    
    //Pikmin born so far.
    MISSION_METRIC_PIKMIN_BORN,
    
    //Pikmin lost so far.
    MISSION_METRIC_PIKMIN_LOST,
    
    //Leaders lost so far.
    MISSION_METRIC_LEADERS_LOST,
    
    //Object collection points.
    MISSION_METRIC_OBJECT_COLLECTION_PTS,
    
    //Treasure collection points.
    MISSION_METRIC_TREASURE_COLLECTION_PTS,
    
    //Enemy collection points.
    MISSION_METRIC_ENEMY_COLLECTION_PTS,
    
    //Enemy defeat points.
    MISSION_METRIC_ENEMY_DEFEAT_PTS,
    
};


//Possible end conditions for missions.
enum MISSION_END_COND {

    //Ended early through the pause menu.
    MISSION_END_COND_PAUSE_MENU,
    
    //A metric reached an amount or greater.
    MISSION_END_COND_METRIC_OR_MORE,
    
    //A metric reached an amount or lesser.
    MISSION_END_COND_METRIC_OR_LESS,
    
    //A leader took damage.
    MISSION_END_COND_TAKE_DAMAGE,
    
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


//Mission mob group type enum naming.
buildEnumNames(missionMobGroupTypeNames, MISSION_MOB_GROUP)({
    { MISSION_MOB_GROUP_CUSTOM, "Custom" },
    { MISSION_MOB_GROUP_TREASURES, "Treasures" },
    { MISSION_MOB_GROUP_ENEMIES, "Enemies" },
    { MISSION_MOB_GROUP_TREASURES_ENEMIES, "Treasures and enemies" },
    { MISSION_MOB_GROUP_LEADERS, "Leaders" },
    { MISSION_MOB_GROUP_PIKMIN, "Pikmin" },
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


//Possible types of ways to display something in a mission HUD item.
enum MISSION_HUD_ITEM_DISPLAY {

    //Just text.
    MISSION_HUD_ITEM_DISPLAY_TEXT,
    
    //A clock that ticks down from a certain amount to 0.
    MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN,
    
    //The clock that ticks up endlessly.
    MISSION_HUD_ITEM_DISPLAY_CLOCK_UP,
    
    //A score counter and ruler.
    MISSION_HUD_ITEM_DISPLAY_SCORE,
    
    //A health bar.
    MISSION_HUD_ITEM_DISPLAY_HEALTH,
    
    //The current amount of something, out of a total.
    MISSION_HUD_ITEM_DISPLAY_CUR_TOT,
    
    //The remaining amount of something, out of a total.
    MISSION_HUD_ITEM_DISPLAY_REM_TOT,
    
    //The current amount of something.
    MISSION_HUD_ITEM_DISPLAY_CUR,
    
    //The remaining amount of something.
    MISSION_HUD_ITEM_DISPLAY_REM,
    
    //The total amount of something.
    MISSION_HUD_ITEM_DISPLAY_TOT,
    
};


//Mission HUD item display type enum naming.
buildEnumNames(missionHudItemDisplayTypeNames, MISSION_HUD_ITEM_DISPLAY)({
    { MISSION_HUD_ITEM_DISPLAY_TEXT, "Custom text" },
    { MISSION_HUD_ITEM_DISPLAY_CLOCK_DOWN, "Clock ticking down" },
    { MISSION_HUD_ITEM_DISPLAY_CLOCK_UP, "Clock ticking up" },
    { MISSION_HUD_ITEM_DISPLAY_SCORE, "Score" },
    { MISSION_HUD_ITEM_DISPLAY_HEALTH, "Health bar" },
    { MISSION_HUD_ITEM_DISPLAY_CUR_TOT, "Current amount / total" },
    { MISSION_HUD_ITEM_DISPLAY_REM_TOT, "Remaining amount / total" },
    { MISSION_HUD_ITEM_DISPLAY_CUR, "Current amount" },
    { MISSION_HUD_ITEM_DISPLAY_REM, "Remaining amount" },
    { MISSION_HUD_ITEM_DISPLAY_TOT, "Total amount" },
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
enum MISSION_GOAL_OLD {

    //The player plays until they end from the pause menu.
    MISSION_GOAL_OLD_END_MANUALLY,
    
    //The player must collect certain treasures, or all of them.
    MISSION_GOAL_OLD_COLLECT_TREASURE,
    
    //The player must defeat certain enemies, or all of them.
    MISSION_GOAL_OLD_BATTLE_ENEMIES,
    
    //The player must survive for a certain amount of time.
    MISSION_GOAL_OLD_TIMED_SURVIVAL,
    
    //The player must get a leader or all of them to the exit point.
    MISSION_GOAL_OLD_GET_TO_EXIT,
    
    //The player must grow enough Pikmin to reach a certain total.
    MISSION_GOAL_OLD_GROW_PIKMIN,
    
};


//Mission goal enum naming.
buildEnumNames(missionGoalNames, MISSION_GOAL_OLD)({
    { MISSION_GOAL_OLD_END_MANUALLY, "End whenever you want" },
    { MISSION_GOAL_OLD_COLLECT_TREASURE, "Collect treasures" },
    { MISSION_GOAL_OLD_BATTLE_ENEMIES, "Battle enemies" },
    { MISSION_GOAL_OLD_TIMED_SURVIVAL, "Survive" },
    { MISSION_GOAL_OLD_GET_TO_EXIT, "Get to the exit" },
    { MISSION_GOAL_OLD_GROW_PIKMIN, "Grow Pikmin" },
});


//DEPRECATED by MISSION_END_COND in 1.2.0.
//Possible ways to fail at a mission.
enum MISSION_FAIL_COND_OLD {

    //Reaching the time limit.
    MISSION_FAIL_COND_OLD_TIME_LIMIT,
    
    //Reaching a certain Pikmin amount or fewer. 0 = total extinction.
    MISSION_FAIL_COND_OLD_TOO_FEW_PIKMIN,
    
    //Reaching a certain Pikmin amount or more.
    MISSION_FAIL_COND_OLD_TOO_MANY_PIKMIN,
    
    //Losing a certain amount of Pikmin.
    MISSION_FAIL_COND_OLD_LOSE_PIKMIN,
    
    //A leader takes damage.
    MISSION_FAIL_COND_OLD_TAKE_DAMAGE,
    
    //Losing a certain amount of leaders.
    MISSION_FAIL_COND_OLD_LOSE_LEADERS,
    
    //Defeating a certain amount of enemies.
    MISSION_FAIL_COND_OLD_DEFEAT_ENEMIES,
    
    //Ending from the pause menu.
    MISSION_FAIL_COND_OLD_PAUSE_MENU,
    
};


//DEPRECATED in 1.2.0 by MISSION_SCORE_CRITERION.
//Possible criteria for a mission's point scoring.
enum MISSION_SCORE_CRITERIA_OLD {

    //Points per Pikmin born.
    MISSION_SCORE_CRITERIA_OLD_PIKMIN_BORN,
    
    //Points per Pikmin death.
    MISSION_SCORE_CRITERIA_OLD_PIKMIN_DEATH,
    
    //Points per second left. Only for missions with a time limit.
    MISSION_SCORE_CRITERIA_OLD_SEC_LEFT,
    
    //Points per second passed.
    MISSION_SCORE_CRITERIA_OLD_SEC_PASSED,
    
    //Points per treasure point.
    MISSION_SCORE_CRITERIA_OLD_TREASURE_POINTS,
    
    //Points per enemy defeat point.
    MISSION_SCORE_CRITERIA_OLD_ENEMY_POINTS,
    
};


#pragma endregion
#pragma region General classes


/**
 * @brief Represents an end condition definition in the mission.
 */
struct MissionEndCond {

    //--- Public members ---
    
    //Type of condition.
    MISSION_END_COND type = MISSION_END_COND_PAUSE_MENU;
    
    //Type of metric, if applicable.
    MISSION_METRIC metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
    
    //Mob group, region, or script slot index for the metric, if applicable.
    size_t idxParam = 0;
    
    //Match amount parameter, if applicable.
    //INVALID means it should let the metric type calculate it.
    size_t matchAmount = 1;
    
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
    
    
    //--- Public function declarations ---
    
    bool usesMetric() const;
    
};


/**
 * @brief Definition of a group of mobs that is relevant to a mission.
 */
struct MissionMobGroup {

    //--- Public members ---
    
    //Type.
    MISSION_MOB_GROUP type = MISSION_MOB_GROUP_CUSTOM;
    
    //For enemies, do they need to be collected, or is it enough for them
    //to be defeated?
    bool enemiesNeedCollection = false;
    
    //Whether it should be highlighted on the radar.
    bool highlightOnRadar = true;
    
    //List of mob indexes, if applicable.
    vector<size_t> mobIdxs;
    
    
    //--- Public function declarations ---
    
    vector<size_t> calculateList() const;
    float calculateTotalHealth() const;
    
};


/**
 * @brief One rule definition for how the score is determined.
 */
struct MissionScoreCriterion {

    //--- Public members ---
    
    //Type.
    MISSION_METRIC metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
    
    //Mob group, region, or script slot index for the metric, if applicable.
    size_t idxParam = 0;
    
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
    
    //Type of display.
    MISSION_HUD_ITEM_DISPLAY displayType = MISSION_HUD_ITEM_DISPLAY_TEXT;
    
    //Metric, if applicable.
    MISSION_METRIC metricType = MISSION_METRIC_MOB_GROUP_CLEARED_MOBS;
    
    //Mob group, region, or script slot index for the metric, if applicable.
    size_t idxParam = 0;
    
    //Text to show, if applicable.
    string text;
    
    //Fixed number for the total amount, if applicable.
    //INVALID means it should let the metric type calculate it.
    size_t totalAmount = INVALID;
    
    
    //--- Public function declarations ---
    
    bool usesMetric() const;
    bool usesText() const;
    bool usesTotal() const;
    
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
    void applyPresetBattleEnemies();
    void applyPresetCollectEverything();
    void applyPresetCollectPikmin();
    void applyPresetCollectTreasure();
    void applyPresetDefeatBosses();
    string getBriefingObjectiveText() const;
    vector<string> getNoteBulletPoints() const;
    vector<string> getMedalAwardBulletPoints() const;
    MISSION_MEDAL getScoreMedal(int score);
    bool isPauseMenuEndClear() const;
    void reset();
    
};


/**
 * @brief Info about a given mission's best attempt record.
 */
struct MissionRecord {

    //--- Public members ---
    
    //Score obtained.
    int score = 0;
    
    //Date of the record.
    string date;
    
    
    //--- Public function declarations ---
    
    void clear();
    bool loadFromDataNode(DataNode* node, bool* ported = nullptr);
    bool saveToDataNode(DataNode* node);
    bool isPlatinum(const MissionData& mission);
    
};


#pragma endregion
#pragma region Metric types


/**
 * @brief Represents a type of metric about what is going on in the mission.
 */
class MissionMetricType {

public:

    //--- Public misc. definitions ---
    
    /**
     * @brief Static information about this type.
     */
    struct Info {
    
        //Its name.
        string name;
        
        //Friendlier player-facing name.
        string friendlyName;
        
        //Descriptor of the index parameter, if any.
        string idxParamName;
        
        //Description of the index parameter, if any.
        string idxParamDescription;
        
        //Whether it can automatically calculate a target amount.
        bool hasAutoTarget = false;
        
    };
    
    
    //--- Public function declarations ---
    
    virtual ~MissionMetricType() = default;
    virtual Info getInfo() const = 0;
    virtual int getAmount(size_t idxParam = 0) const = 0;
    int getRemaining(size_t idxParam = 0, size_t manualTarget = 0) const;
    int getTarget(size_t idxParam = 0, size_t manualTarget = 0) const;
    virtual bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const = 0;
    
    
protected:

    //--- Protected function declarations ---
    
    virtual int getAutoTarget(size_t idxParam = 0) const = 0;
    
};


/**
 * @brief Represents the "mob group cleared mobs" metric type.
 */
class MissionMetricTypeMobGroup : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "mob group health" metric type.
 */
class MissionMetricTypeMobGroupHealth : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "seconds left" metric type.
 */
class MissionMetricTypeSecsLeft : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "seconds passed" metric type.
 */
class MissionMetricTypeSecsPassed : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "leaders in region" metric type.
 */
class MissionMetricTypeLeadersInRegion : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "living Pikmin" metric type.
 */
class MissionMetricTypeLivingPikmin : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "Pikmin born" metric type.
 */
class MissionMetricTypePikminBorn : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "Pikmin deaths" metric type.
 */
class MissionMetricTypePikminDeaths : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "Pikmin lost" metric type.
 */
class MissionMetricTypeLeadersLost : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "object collection points" metric type.
 */
class MissionMetricTypeObjectCollectionPts : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "treasure collection points" metric type.
 */
class MissionMetricTypeTreasureCollectionPts : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "enemy collection points" metric type.
 */
class MissionMetricTypeEnemyCollectionPts : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "enemy defeat points" metric type.
 */
class MissionMetricTypeDefeatPts : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
};


/**
 * @brief Represents the "script slot" metric type.
 */
class MissionMetricTypeScriptSlot : public MissionMetricType {

public:

    //--- Public function declarations ---
    
    MissionMetricType::Info getInfo() const override;
    int getAmount(size_t idxParam = 0) const override;
    bool getZoomData(
        size_t idxParam, Point* outCamPos, float* outCamZoom
    ) const override;
    
    
protected:

    //--- Protected function declarations ---
    
    int getAutoTarget(size_t idxParam = 0) const override;
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
     * @brief Information about the type.
     */
    struct Info {
    
        //--- Public members ---
        
        //Its name.
        string name;
        
        //Description of the end condition type.
        string description;
        
    };
    
    
    //--- Public function declarations ---
    
    virtual ~MissionEndCondType() = default;
    virtual Info getInfo() const = 0;
    virtual bool getZoomData(
        MissionEndCond* cond, Point* outCamPos, float* outCamZoom
    ) const = 0;
    virtual bool isMet(MissionEndCond* cond) const = 0;
    
};


/**
 * @brief Class representing the "pause menu"
 * mission end condition.
 */
class MissionEndCondTypePauseMenu : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    Info getInfo() const override;
    bool getZoomData(
        MissionEndCond* cond, Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(MissionEndCond* cond) const override;
    
};


/**
 * @brief Class representing the "metric or more"
 * mission end condition.
 */
class MissionEndCondTypeMetricOrMore : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    Info getInfo() const override;
    bool getZoomData(
        MissionEndCond* cond, Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(MissionEndCond* cond) const override;
    
};


/**
 * @brief Class representing the "metric or less"
 * mission end condition.
 */
class MissionEndCondTypeMetricOrLess : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    Info getInfo() const override;
    bool getZoomData(
        MissionEndCond* cond, Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(MissionEndCond* cond) const override;
    
};


/**
 * @brief Class representing the "take damage"
 * mission end condition.
 */
class MissionEndCondTypeTakeDamage : public MissionEndCondType {

public:

    //--- Public function declarations ---
    
    Info getInfo() const override;
    bool getZoomData(
        MissionEndCond* cond, Point* outCamPos, float* outCamZoom
    ) const override;
    bool isMet(MissionEndCond* cond) const override;
    
};


#pragma endregion
