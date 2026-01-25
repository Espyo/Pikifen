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

#include <string>
#include <unordered_set>

#include "../../core/const.h"
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


/**
 * @brief Info about a given area's mission.
 */
struct MissionDataOld {

    //--- Members ---
    
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
    
    
    //--- Function declarations ---
    
    MISSION_MEDAL getScoreMedal(int score);
    
};


/**
 * @brief Info about a given mission's record.
 */
struct MissionRecord {

    //--- Members ---
    
    //Has the mission's goal been cleared?
    bool clear = false;
    
    //Score obtained.
    int score = 0;
    
    //Date of the record.
    string date;
    
    
    //--- Function declarations ---
    
    bool isPlatinum(const MissionDataOld& mission);
    
};


/**
 * @brief Class interface for a mission fail condition.
 */
class MissionFail {

public:

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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

    //--- Function declarations ---
    
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
#pragma region Score criteria


/**
 * @brief Class interface for a mission score criterion.
 */
class MissionScoreCriterion {

public:

    //--- Function declarations ---
    
    virtual ~MissionScoreCriterion() = default;
    virtual string getName() const = 0;
    virtual int getMultiplier(MissionDataOld* mission) const = 0;
    virtual int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const = 0;
    
};


/**
 * @brief Class representing the "enemy points" mission score criterion.
 */
class MissionScoreCriterionEnemyPoints : public MissionScoreCriterion {

public:

    //--- Function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin born" mission score criterion.
 */
class MissionScoreCriterionPikminBorn : public MissionScoreCriterion {

public:

    //--- Function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "Pikmin death" mission score criterion.
 */
class MissionScoreCriterionPikminDeath : public MissionScoreCriterion {

public:

    //--- Function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "seconds left" mission score criterion.
 */
class MissionScoreCriterionSecLeft : public MissionScoreCriterion {

public:

    //--- Function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "seconds passed" mission score criterion.
 */
class MissionScoreCriterionSecPassed : public MissionScoreCriterion {

public:

    //--- Function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};


/**
 * @brief Class representing the "treasure points" mission score criterion.
 */
class MissionScoreCriterionTreasurePoints : public MissionScoreCriterion {

public:

    //--- Function declarations ---
    
    string getName() const override;
    int getMultiplier(MissionDataOld* mission) const override;
    int getScore(
        GameplayState* gameplay, MissionDataOld* mission
    ) const override;
    
};
