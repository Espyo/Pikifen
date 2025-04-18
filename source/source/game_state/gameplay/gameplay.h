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
    
    //Closest to player 1's leader, for the previous, current, next type.
    Mob* closestGroupMember[3] = { nullptr, nullptr, nullptr };
    
    //Is the group member closest to player 1's leader distant?
    bool closestGroupMemberDistant = false;
    
    //Index of player 1's current leader, in the array of available leaders.
    size_t curLeaderIdx = 0;
    
    //Pointer to player 1's leader. Cache for convenience.
    Leader* curLeaderPtr = nullptr;
    
    //What day it is, in-game.
    size_t day = 1;
    
    //What time of the day is it in-game? In minutes.
    float dayMinutes = 0.0f;
    
    //Multiply the delta_t by this much. Only affects gameplay stuff, not menus.
    float deltaTMult = 1.0f;
    
    //Replay of the gameplay.
    Replay gameplayReplay;
    
    //How many seconds of actual playtime. Only counts on player control.
    float gameplayTimePassed = 0.0f;
    
    //Information about the in-game HUD.
    Hud* hud = nullptr;
    
    //Position of the last enemy killed. LARGE_FLOAT for none.
    Point lastEnemyKilledPos;
    
    //Position of the last leader to get hurt. LARGE_FLOAT for none.
    Point lastHurtLeaderPos;
    
    //Position of the last Pikmin born. LARGE_FLOAT for none.
    Point lastPikminBornPos;
    
    //Position of the last Pikmin that died. LARGE_FLOAT for none.
    Point lastPikminDeathPos;
    
    //Position of the last ship that got a treasure. LARGE_FLOAT for none.
    Point lastShipThatGotTreasurePos;
    
    //Player 1's leader cursor, in window coordinates.
    Point leaderCursorWin;
    
    //Sector that player 1's leader cursor is on, if any.
    Sector* leaderCursorSector = nullptr;
    
    //Player 1's leader cursor, in world coordinates.
    Point leaderCursorW;
    
    //List of all mobs in the area.
    MobLists mobs;
    
    //Information about the message box currently active on player 1, if any.
    GameplayMessageBox* msgBox = nullptr;
    
    //ID of the next mob to be created.
    size_t nextMobId = 0;
    
    //Current notification.
    Notification notification;
    
    //Manager of all particles.
    ParticleManager particles;
    
    //Path manager.
    PathManager pathMgr;
    
    //Path of the folder of the area to be loaded.
    string pathOfAreaToLoad;
    
    //All droplets of precipitation.
    vector<Point> precipitation;
    
    //Time until the next drop of precipitation.
    Timer precipitationTimer;
    
    //Spray that player 1 has currently selected.
    size_t selectedSpray = 0;
    
    //How many of each spray/ingredients player 1 has.
    vector<SprayStats> sprayStats;
    
    //All types of subgroups.
    SubgroupTypeManager subgroupTypes;
    
    //Angle at which player 1 is swarming.
    float swarmAngle = 0.0f;
    
    //General intensity of player 1's swarm in the specified angle.
    float swarmMagnitude = 0.0f;
    
    //Destination of player 1's throw.
    Point throwDest;
    
    //Mob that player 1's throw will land on, if any.
    Mob* throwDestMob = nullptr;
    
    //Sector that player 1's throw will land on, if any.
    Sector* throwDestSector = nullptr;
    
    //Are we currently loading the gameplay state?
    bool loading = false;
    
    //Are we currently unloading the gameplay state?
    bool unloading = false;
    
    //Have we went to the results screen yet?
    bool wentToResults = false;
    
    //Information about player 1's whistle.
    Whistle whistle;
    
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
    
    //How many enemy deaths so far.
    size_t enemyDeaths = 0;
    
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
    float scoreIndicator = 0.0f;
    
    //Current interlude, if any.
    INTERLUDE curInterlude = INTERLUDE_NONE;
    
    //Time passed in the current interlude.
    float interludeTime = 0.0f;
    
    //Current big message, if any.
    BIG_MESSAGE curBigMsg = BIG_MESSAGE_NONE;
    
    //Time passed in the current big message.
    float bigMsgTime = 0.0f;
    
    //Current state of the boss music.
    BOSS_MUSIC_STATE bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    
    //Zoom level to use on the radar.
    float radarZoom = PAUSE_MENU::RADAR_DEF_ZOOM;
    
    //Number of Pikmin born so far, per type.
    map<PikminType*, long> pikminBornPerType;
    
    //Number of Pikmin lost so far, per type.
    map<PikminType*, long> pikminDeathsPerType;
    
    
    //--- Function declarations ---
    
    ALLEGRO_BITMAP* drawToBitmap(
        const MakerTools::AreaImageSettings &settings
    );
    void enter();
    void leave(const GAMEPLAY_LEAVE_TARGET target);
    void startLeaving(const GAMEPLAY_LEAVE_TARGET target);
    void changeSprayCount(size_t type_idx, signed int amount);
    size_t getAmountOfFieldPikmin(const PikminType* filter = nullptr);
    size_t getAmountOfGroupPikmin(const PikminType* filter = nullptr);
    size_t getAmountOfIdlePikmin(const PikminType* filter = nullptr);
    long getAmountOfOnionPikmin(const PikminType* filter = nullptr);
    long getAmountOfTotalPikmin(const PikminType* filter = nullptr);
    void isNearEnemyAndBoss(bool* near_enemy, bool* near_boss);
    void updateAvailableLeaders();
    void updateClosestGroupMembers();
    void load() override;
    void unload() override;
    void handleAllegroEvent(ALLEGRO_EVENT &ev) override;
    void doLogic() override;
    void doDrawing() override;
    void updateTransformations() override;
    string getName() const override;
    
private:

    //--- Members ---
    
    //Points to an interactable close enough for player 1 to use, if any.
    Interactable* closeToInteractableToUse = nullptr;
    
    //Points to a nest-like object close enough for player 1 to open, if any.
    PikminNest* closeToNestToOpen = nullptr;
    
    //Points to a Pikmin close enough for player 1 to pluck, if any.
    Pikmin* closeToPikminToPluck = nullptr;
    
    //Points to a ship close enough for player 1 to heal in, if any.
    Ship* closeToShipToHeal = nullptr;
    
    //Ligthten player 1's cursor by this due to leader/cursor height difference.
    float cursorHeightDiffLight = 0.0f;
    
    //Movement of player 1's cursor via non-mouse means.
    MovementInfo cursorMovement;
    
    //Is input enabled, for reasons outside the readyForInput variable?
    bool isInputAllowed = false;
    
    //Bitmap that lights up the area when in blackout mode.
    ALLEGRO_BITMAP* lightmapBmp = nullptr;
    
    //Movement of player 1's leader.
    MovementInfo leaderMovement;
    
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
    
    //Is player 1 holding the "swarm to cursor" button?
    bool swarmCursor = false;
    
    //Reach of player 1's swarm.
    MovementInfo swarmMovement;
    
    
    //--- Function declarations ---
    
    void doAestheticLeaderLogic(float delta_t);
    void doAestheticLogic(float delta_t);
    void doGameDrawing(
        ALLEGRO_BITMAP* bmp_output = nullptr,
        const ALLEGRO_TRANSFORM* bmp_transform = nullptr,
        const MakerTools::AreaImageSettings &bmp_settings =
            MakerTools::AreaImageSettings()
    );
    void doGameplayLeaderLogic(float delta_t);
    void doGameplayLogic(float delta_t);
    void doMenuLogic();
    void drawBackground(ALLEGRO_BITMAP* bmp_output);
    void drawDebugTools();
    void drawLeaderCursor(const ALLEGRO_COLOR &color);
    void drawIngameText();
    void drawBigMsg();
    void drawLightingFilter();
    void drawGameplayMessageBox();
    void drawOnionMenu();
    void drawPauseMenu();
    void drawPrecipitation();
    void drawSystemStuff();
    void drawThrowPreview();
    void drawTreeShadows();
    void drawWorldComponents(ALLEGRO_BITMAP* bmp_output);
    void endMission(bool cleared);
    ALLEGRO_BITMAP* generateFogBitmap(
        float near_radius, float far_radius
    );
    Mob* getClosestGroupMember(const SubgroupType* type, bool* distant = nullptr);
    void handlePlayerAction(const PlayerAction &action);
    void initHud();
    bool isMissionClearMet();
    bool isMissionFailMet(MISSION_FAIL_COND* reason);
    void loadGameContent();
    void markAreaCellsActive(
        const Point &top_left, const Point &bottom_right
    );
    void markAreaCellsActive(
        int from_x, int to_x, int from_y, int to_y
    );
    void processMobInteractions(Mob* m_ptr, size_t m);
    void processMobMiscInteractions(
        Mob* m_ptr, Mob* m2_ptr, size_t m, size_t m2,
        const Distance &d, const Distance &d_between,
        vector<PendingIntermobEvent> &pending_intermob_events
    );
    void processMobReaches(
        Mob* m_ptr, Mob* m2_ptr, size_t m, size_t m2, const Distance &d_between,
        vector<PendingIntermobEvent> &pending_intermob_events
    );
    void processMobTouches(
        Mob* m_ptr, Mob* m2_ptr, size_t m, size_t m2, Distance &d
    );
    bool shouldIngorePlayerAction(const PlayerAction &action);
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
    
    GameplayMessageBox(const string &text, ALLEGRO_BITMAP* speaker_icon);
    void advance();
    void close();
    void tick(float delta_t);
    
};
