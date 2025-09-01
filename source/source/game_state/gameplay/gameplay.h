/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gameplay state class and
 * gameplay state-related functions.
 */

#pragma once

#include "../../content/area/mission.h"
#include "../../content/mob/interactable.h"
#include "../../content/mob/onion.h"
#include "../../content/mob/pikmin.h"
#include "../../content/mob/ship.h"
#include "../../content/other/gui.h"
#include "../../core/controls_mediator.h"
#include "../../core/maker_tools.h"
#include "../../core/player.h"
#include "../../core/replay.h"
#include "../../util/general_utils.h"
#include "../game_state.h"
#include "gameplay_utils.h"
#include "hud.h"
#include "onion_menu.h"
#include "pause_menu.h"


namespace GAMEPLAY {
extern const float AREA_INTRO_HUD_MOVE_TIME;
extern const float AREA_TITLE_FADE_DURATION;
extern const float BIG_MSG_GO_DUR;
extern const string BIG_MSG_GO_TEXT;
extern const float BIG_MSG_MISSION_CLEAR_DUR;
extern const string BIG_MSG_MISSION_CLEAR_TEXT;
extern const float BIG_MSG_MISSION_FAILED_DUR;
extern const string BIG_MSG_MISSION_FAILED_TEXT;
extern const float BIG_MSG_ONE_MIN_LEFT_DUR;
extern const string BIG_MSG_ONE_MIN_LEFT_TEXT;
extern const float BIG_MSG_READY_DUR;
extern const string BIG_MSG_READY_TEXT;
extern const float BOSS_MUSIC_DISTANCE;
extern const float CAMERA_BOX_MARGIN;
extern const float CAMERA_SMOOTHNESS_MULT;
extern const unsigned char COLLISION_OPACITY;
extern const float ENEMY_MIX_DISTANCE;
extern const int FOG_BITMAP_SIZE;
extern const float LEADER_LAND_PART_MAX_SIZE;
extern const float LEADER_LAND_PART_SIZE_MULT;
extern const float MENU_ENTRY_HUD_MOVE_TIME;
extern const float MENU_EXIT_HUD_MOVE_TIME;
extern const unsigned char PREVIEW_OPACITY;
extern const float PREVIEW_TEXTURE_SCALE;
extern const float PREVIEW_TEXTURE_TIME_MULT;
extern const float REPLAY_SAVE_FREQUENCY;
extern const float TREE_SHADOW_SWAY_AMOUNT;
extern const float TREE_SHADOW_SWAY_SPEED;
extern const float MENU_ENTRY_HUD_MOVE_TIME;
extern const float MENU_EXIT_HUD_MOVE_TIME;
extern const float SWARM_ARROW_SPEED;
}


//Types of interludes -- stuff before or after gameplay proper in the area.
enum INTERLUDE {

    //None.
    INTERLUDE_NONE,
    
    //Ready?
    INTERLUDE_READY,
    
    //Mission end, be it due to a clear or a fail.
    INTERLUDE_MISSION_END,
    
};


//Types of big messages -- text that appears in large letters on the HUD.
enum BIG_MESSAGE {

    //None.
    BIG_MESSAGE_NONE,
    
    //Ready?
    BIG_MESSAGE_READY,
    
    //Go!
    BIG_MESSAGE_GO,
    
    //1 minute left!
    BIG_MESSAGE_ONE_MIN_LEFT,
    
    //Mission clear!
    BIG_MESSAGE_MISSION_CLEAR,
    
    //Mission failed...
    BIG_MESSAGE_MISSION_FAILED,
    
};


//States for the area's boss music.
enum BOSS_MUSIC_STATE {

    //Hasn't played yet.
    BOSS_MUSIC_STATE_NEVER_PLAYED,
    
    //Is playing.
    BOSS_MUSIC_STATE_PLAYING,
    
    //Is not playing right now, but has played before.
    BOSS_MUSIC_STATE_PAUSED,
    
    //Playing the victory theme.
    BOSS_MUSIC_STATE_VICTORY,
};


struct GameplayMessageBox;


/**
 * @brief Basic information about the current interlude.
 */
struct InterludeInfo {

    //--- Function declarations ---
    
    void set(INTERLUDE id, bool instantVolumeChange);
    INTERLUDE get();
    float getTime();
    void overrideTime(float time);
    void tick(float delta_t);
    
    
    private:
    
    //--- Members ---
    
    //ID of the current interlude, if any.
    INTERLUDE curId = INTERLUDE_NONE;
    
    //Time passed in the current interlude.
    float curTime = 0.0f;
    
};


/**
 * @brief Basic information about the current big message.
 */
struct BigMessageInfo {

    //--- Function declarations ---
    
    void set(BIG_MESSAGE id);
    BIG_MESSAGE get();
    float getTime();
    void overrideTime(float time);
    void tick(float delta_t);
    
    
    private:
    
    //--- Members ---
    
    //ID of the current big message, if any.
    BIG_MESSAGE curId = BIG_MESSAGE_NONE;
    
    //Time passed in the current big message.
    float curTime = 0.0f;
    
};


/**
 * @brief Standard gameplay state. This is where the action happens.
 */
class GameplayState : public GameState {

public:

    //--- Members ---
    
    //Is the player playing after hours?
    bool afterHours = false;
    
    //Divides the area into cells, and lists whether a given cell is
    //active for this frame or not.
    vector<vector<bool> > areaActiveCells;
    
    //How many seconds since area load. Doesn't count when things are paused.
    float areaTimePassed = 0.0f;
    
    //Timer used to fade out the area's title when the area is entered.
    Timer areaTitleFadeTimer = Timer(GAMEPLAY::AREA_TITLE_FADE_DURATION);
    
    //Leaders available to control.
    vector<Leader*> availableLeaders;
    
    //Fog effect buffer.
    ALLEGRO_BITMAP* bmpFog = nullptr;
    
    //What day it is, in-game.
    size_t day = 1;
    
    //What time of the day is it in-game? In minutes.
    float dayMinutes = 0.0f;
    
    //Multiply the deltaT by this much. Only affects gameplay stuff, not menus.
    float deltaTMult = 1.0f;
    
    //Replay of the gameplay.
    Replay gameplayReplay;
    
    //How many seconds of actual playtime. Only counts on player control.
    float gameplayTimePassed = 0.0f;
    
    //Position of the last enemy defeated. LARGE_FLOAT for none.
    Point lastEnemyDefeatedPos;
    
    //Position of the last leader to get hurt. LARGE_FLOAT for none.
    Point lastHurtLeaderPos;
    
    //Position of the last Pikmin born. LARGE_FLOAT for none.
    Point lastPikminBornPos;
    
    //Position of the last Pikmin that died. LARGE_FLOAT for none.
    Point lastPikminDeathPos;
    
    //Position of the last ship that got a treasure. LARGE_FLOAT for none.
    Point lastShipThatGotTreasurePos;
    
    //List of all mobs in the area.
    MobLists mobs;
    
    //Information about the message box currently active, if any.
    GameplayMessageBox* msgBox = nullptr;
    
    //ID of the next mob to be created.
    size_t nextMobId = 0;
    
    //Manager of all particles.
    ParticleManager particles;
    
    //Path manager.
    PathManager pathMgr;
    
    //Path of the folder of the area to be loaded.
    string pathOfAreaToLoad;
    
    //Possible teams for the players.
    PlayerTeam playerTeams[MAX_PLAYER_TEAMS];
    
    //Players that are participating.
    vector<Player> players;
    
    //All droplets of precipitation.
    vector<Point> precipitation;
    
    //Time until the next drop of precipitation.
    Timer precipitationTimer;
    
    //All types of subgroups.
    SubgroupTypeManager subgroupTypes;
    
    //Are we currently loading the gameplay state?
    bool loading = false;
    
    //Are we currently unloading the gameplay state?
    bool unloading = false;
    
    //Have we went to the results screen yet?
    bool wentToResults = false;
    
    //IDs of mobs remaining for the current mission goal, if applicable.
    unordered_set<size_t> missionRemainingMobIds;
    
    //How many mobs are required for the mission goal. Cache for convenience.
    size_t missionRequiredMobAmount = 0;
    
    //How many Pikmin were born so far.
    size_t pikminBorn = 0;
    
    //How many Pikmin deaths so far.
    size_t pikminDeaths = 0;
    
    //How many treasures collected so far.
    size_t treasuresCollected = 0;
    
    //How many treasures exist in total.
    size_t treasuresTotal = 0;
    
    //How many mission goal treasures were collected so far.
    size_t goalTreasuresCollected = 0;
    
    //How many mission goal treasures exist in total.
    size_t goalTreasuresTotal = 0;
    
    //How many treasure points collected so far.
    size_t treasurePointsCollected = 0;
    
    //How many treasure points exist in total.
    size_t treasurePointsTotal = 0;
    
    //How many enemy defeats so far.
    size_t enemyDefeats = 0;
    
    //How many enemies exist in total.
    size_t enemyTotal = 0;
    
    //How many enemy points collected so far.
    size_t enemyPointsCollected = 0;
    
    //How many enemy points exist in total.
    size_t enemyPointsTotal = 0;
    
    //Reason for mission fail, if any. INVALID for none.
    MISSION_FAIL_COND missionFailReason = (MISSION_FAIL_COND) INVALID;
    
    //Current mission score, for use in the HUD.
    int missionScore = 0;
    
    //Mission score in the previous frame.
    int oldMissionScore = 0;
    
    //GUI item with the mission score's text.
    GuiItem* missionScoreCurText = nullptr;
    
    //Mission goal current count in the previous frame.
    int oldMissionGoalCur = 0;
    
    //GUI item with the mission goal current count's text.
    GuiItem* missionGoalCurText = nullptr;
    
    //Mission goal primary fail condition count in the previous frame.
    int oldMissionFail1Cur = 0;
    
    //GUI item with the mission primary fail condition current count's text.
    GuiItem* missionFail1CurText = nullptr;
    
    //Mission goal secondary fail condition count in the previous frame.
    int oldMissionFail2Cur = 0;
    
    //GUI item with the mission secondary fail condition current count's text.
    GuiItem* missionFail2CurText = nullptr;
    
    //How many leaders are in the mission exit. Cache for convenience.
    size_t curLeadersInMissionExit = 0;
    
    //Current number of living leaders. Cache for convenience.
    size_t nrLivingLeaders = 0;
    
    //How many leaders have been lost so far. Cache for convenience.
    size_t leadersKod = 0;
    
    //Starting number of leader mobs.
    size_t startingNrOfLeaders = 0;
    
    //Ratio of the mission goal HUD item's indicator.
    float goalIndicatorRatio = 0.0f;
    
    //Ratio of the mission primary fail condition HUD item's indicator.
    float fail1IndicatorRatio = 0.0f;
    
    //Ratio of the mission secondary fail condition HUD item's indicator.
    float fail2IndicatorRatio = 0.0f;
    
    //Position of the mission score HUD item's indicator.
    float scoreFlapper = 0.0f;
    
    //Animation timer for the "Got it!" medal text on the mission score ruler.
    float medalGotItJuiceTimer = 0.0f;
    
    //Current interlude info.
    InterludeInfo interlude;
    
    //Current big message info.
    BigMessageInfo bigMsg;
    
    //Current state of the boss music.
    BOSS_MUSIC_STATE bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    
    //Number of Pikmin born so far, per type.
    map<PikminType*, long> pikminBornPerType;
    
    //Number of Pikmin lost so far, per type.
    map<PikminType*, long> pikminDeathsPerType;
    
    //Last Pikmin type chosen to resolve a carrying tie, if any.
    PikminType* lastCarryingTieBreaker = nullptr;

    //Lines of the text to show for the print debug mob script action.
    vector<string> printActionLogLines;
    
    //Closest, middle, and farthest zoom levels. Cache for convenience.
    float zoomLevels[3] = { 0.0f, 0.0f, 0.0f };
    
    
    //--- Function declarations ---
    
    ALLEGRO_BITMAP* drawToBitmap(
        const MakerTools::AreaImageSettings& settings
    );
    void enter();
    void leave(const GAMEPLAY_LEAVE_TARGET target);
    void startLeaving(const GAMEPLAY_LEAVE_TARGET target);
    void changeSprayCount(PlayerTeam* team, size_t typeIdx, signed int amount);
    size_t getAmountOfFieldPikmin(const PikminType* filter = nullptr);
    size_t getAmountOfGroupPikmin(
        Player* player, const PikminType* filter = nullptr
    );
    size_t getAmountOfIdlePikmin(const PikminType* filter = nullptr);
    long getAmountOfOnionPikmin(const PikminType* filter = nullptr);
    long getAmountOfTotalPikmin(const PikminType* filter = nullptr);
    void isNearEnemyAndBoss(bool* nearEnemy, bool* nearBoss);
    void updateAvailableLeaders();
    void updateClosestGroupMembers(Player* player);
    void load() override;
    void unload() override;
    void handleAllegroEvent(ALLEGRO_EVENT& ev) override;
    void doLogic() override;
    void doDrawing() override;
    string getName() const override;
    
private:

    //--- Members ---
    
    //Is input enabled, for reasons outside the readyForInput variable?
    bool isInputAllowed = false;
    
    //Bitmap that lights up the area when in blackout mode.
    ALLEGRO_BITMAP* lightmapBmp = nullptr;
    
    //Information about the current Onion menu, if any.
    OnionMenu* onionMenu = nullptr;
    
    //Information about the current pause menu, if any.
    PauseMenu* pauseMenu = nullptr;
    
    //Is the gameplay paused?
    bool paused = false;
    
    //The first frame shouldn't allow for input just yet, because
    //some things are still being set up within the first logic loop.
    //So forbid input until the second frame.
    bool readyForInput = false;
    
    //Timer for the next replay state save.
    Timer replayTimer;
    
    
    //--- Function declarations ---
    
    void doAestheticLeaderLogic(Player* player, float deltaT);
    void doAestheticLogic(float deltaT);
    void doGameDrawing(
        ALLEGRO_BITMAP* bmpOutput = nullptr,
        const ALLEGRO_TRANSFORM* bmpTransform = nullptr,
        const MakerTools::AreaImageSettings& bmpSettings =
            MakerTools::AreaImageSettings()
    );
    void doGameplayLeaderLogic(Player* player, float deltaT);
    void doGameplayLogic(float deltaT);
    void doMenuLogic();
    void doPlayerActionDismiss(Player* player, bool isDown);
    void doPlayerActionLieDown(Player* player, bool isDown);
    void doPlayerActionPause(Player* player, bool isDown, bool radar);
    void doPlayerActionSwitchLeader(Player* player, bool isDown, bool isNext);
    void doPlayerActionSwitchMaturity(Player* player, bool isDown, bool isNext);
    void doPlayerActionSwitchSpray(Player* player, bool isDown, bool isNext);
    void doPlayerActionSwitchType(Player* player, bool isDown, bool isNext);
    void doPlayerActionThrow(Player* player, bool isDown);
    void doPlayerActionToggleZoom(Player* player, bool isDown);
    void doPlayerActionUseCurrentSpray(Player* player, bool isDown);
    void doPlayerActionUseSpray(Player* player, bool isDown, bool second);
    void doPlayerActionWhistle(Player* player, bool isDown);
    void doPlayerActionZoom(Player* player, float inputValue, bool zoomIn);
    void drawBackground(const Viewport& view, ALLEGRO_BITMAP* bmpOutput);
    void drawDebugTools();
    void drawLeaderCursor(Player* player, const ALLEGRO_COLOR& color);
    void drawIngameText(Player* player);
    void drawBigMsg();
    void drawLightingFilter(const Viewport& view);
    void drawGameplayMessageBox();
    void drawOnionMenu();
    void drawPauseMenu();
    void drawPrecipitation();
    void drawThrowPreview(Player* player);
    void drawTreeShadows();
    void drawWorldComponents(const Viewport& view, ALLEGRO_BITMAP* bmpOutput);
    void endMission(bool cleared);
    ALLEGRO_BITMAP* generateFogBitmap(
        float nearRadius, float farRadius
    );
    Mob* getClosestGroupMember(
        Player* player, const SubgroupType* type, bool* distant = nullptr
    );
    Mob* getEnemyOrTreasureOnCursor(Player* player) const;
    void handlePlayerAction(const PlayerAction& action);
    bool isMissionClearMet();
    bool isMissionFailMet(MISSION_FAIL_COND* reason);
    void loadGameContent();
    void markAreaCellsActive(
        const Point& top_left, const Point& bottomRight
    );
    void markAreaCellsActive(
        int fromX, int toX, int fromY, int toY
    );
    void processMobInteractions(Mob* mPtr, size_t m);
    void processMobMiscInteractions(
        Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2,
        const Distance& d, const Distance& dBetween,
        vector<PendingIntermobEvent>& pendingIntermobEvents
    );
    void processMobReaches(
        Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2, const Distance& dBetween,
        vector<PendingIntermobEvent>& pendingIntermobEvents
    );
    void processMobTouches(
        Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2, Distance& d
    );
    bool shouldIgnorePlayerAction(const PlayerAction& action);
    void unloadGameContent();
    void updateAreaActiveCells();
    void updateMobIsActiveFlag();
    
};


/**
 * @brief Info about the current gameplay message box, if any.
 */
struct GameplayMessageBox {

    //--- Members ---
    
    //Full list of message tokens, per line.
    vector<vector<StringToken> > tokensPerLine;
    
    //Icon that represents the speaker, if any.
    ALLEGRO_BITMAP* speakerIcon = nullptr;
    
    //What section of the message are we in?
    size_t curSection = 0;
    
    //What token of the section are we in, for the typing animation?
    size_t curToken = 0;
    
    //From what token did the player perform a typing skip? INVALID = none.
    size_t skippedAtToken = INVALID;
    
    //How long have we been animating tokens for.
    float totalTokenAnimTime = 0.0f;
    
    //How long have we been animating the skipped tokens for.
    float totalSkipAnimTime = 0.0f;
    
    //Timer where the player can't advance. Stops accidental misinputs.
    float misinputProtectionTimer = 0.0f;
    
    //Opacity of the advance button icon.
    float advanceButtonAlpha = 0.0f;
    
    //Time left to swipe the current section away with.
    float swipeTimer = 0.0f;
    
    //Time left in the current transition.
    float transitionTimer = GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    
    //Is it transitioning into view, or out of view?
    bool transitionIn = true;
    
    //Is the message box meant to be deleted?
    bool toDelete = false;
    
    
    //--- Function declarations ---
    
    GameplayMessageBox(const string& text, ALLEGRO_BITMAP* speakerIcon);
    void advance();
    void close();
    void tick(float deltaT);
    
};
