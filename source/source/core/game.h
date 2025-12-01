/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the class that holds everything about the game.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../game_state/game_state.h"

#include "../content/area/area.h"
#include "../content/content_manager.h"
#include "../content/other/mob_script_action.h"
#include "../game_state/animation_editor/editor.h"
#include "../game_state/annex_screen.h"
#include "../game_state/area_editor/editor.h"
#include "../game_state/gameplay/gameplay.h"
#include "../game_state/gui_editor/editor.h"
#include "../game_state/particle_editor/editor.h"
#include "../game_state/results.h"
#include "../game_state/title_screen.h"
#include "../lib/inpution/inpution.h"
#include "audio.h"
#include "game_config.h"
#include "hardware_mediator.h"
#include "misc_structs.h"
#include "modal_gui.h"
#include "options.h"
#include "shaders.h"


namespace GAME {
extern const ALLEGRO_COLOR CURSOR_STANDARD_COLOR;
extern const unsigned char CURSOR_TRAIL_MAX_ALPHA;
extern const float CURSOR_TRAIL_MAX_WIDTH;
extern const float CURSOR_TRAIL_MIN_SPOT_DIFF;
extern const float CURSOR_TRAIL_SAVE_INTERVAL;
extern const unsigned char CURSOR_TRAIL_SAVE_N_SPOTS;
extern const float FADE_DURATION;
extern const float FADE_SLOW_DURATION;
extern const size_t FRAMERATE_AVG_SAMPLE_SIZE;
extern const size_t FRAMERATE_HISTORY_SIZE;
}


/**
 * @brief List of all game states.
 */
struct GameStateList {

    //--- Members ---
    
    //Animation editor.
    AnimationEditor* animationEd = nullptr;
    
    //Area editor.
    AreaEditor* areaEd = nullptr;
    
    //Annex screen for misc. menus.
    AnnexScreen* annexScreen = nullptr;
    
    //Gameplay state.
    GameplayState* gameplay = nullptr;
    
    //GUI editor state.
    GuiEditor* guiEd = nullptr;
    
    //GUI editor state.
    ParticleEditor* particleEd = nullptr;
    
    //Title screen.
    TitleScreen* titleScreen = nullptr;
    
    //Area results screen.
    Results* results = nullptr;
    
    
    //--- Function declarations ---
    
    void init();
    void destroy();
    
};


/**
 * @brief Info about the whole game.
 */
class Game {

public:

    //--- Members ---
    
    //Audio.
    AudioManager audio;
    
    //The error bitmap used to represent bitmaps that were not loaded.
    ALLEGRO_BITMAP* bmpError = nullptr;
    
    //Game's configuration.
    GameConfig config;
    
    //Player controls mediator.
    ControlsMediator controls;
    
    //Hardware mediator.
    HardwareMediator hardware;
    
    //Console.
    Console console;
    
    //Total amount of time the current frame took to process, in seconds.
    double curFrameProcessTime = 0.0f;
    
    //Info about the maker tools.
    MakerTools makerTools;
    
    //Game content.
    ContentManager content;
    
    //Data about the area that's currently being used.
    Area* curAreaData = nullptr;
    
    //Time between the previous frame and the current.
    double deltaT = 0.0f;
    
    //Allegro display that represents the program window.
    ALLEGRO_DISPLAY* display = nullptr;
    
    //A dummy mob state for mobs with no state to use.
    MobState* dummyMobState = nullptr;
    
    //Error manager.
    ErrorManager errors;
    
    //Shader manager.
    ShaderManager shaders;
    
    //Manager for all full-window fade-ins and fade-outs.
    FadeManager fadeMgr = FadeManager(GAME::FADE_DURATION);
    
    //Manager for the system notifications.
    SystemNotificationManager systemNotifications;
    
    //Duration of the last few frames.
    vector<double> framerateHistory;
    
    //Last framerate average started at this point in the history.
    size_t framerateLastAvgPoint = 0.0f;
    
    //Identity matrix transformation. Cache for convenience.
    ALLEGRO_TRANSFORM identityTransform;
    
    //Default Dear ImGui style.
    ImVec4 dearImGuiDefaultStyle[ImGuiCol_COUNT];
    
    //Set to false to stop program execution next frame.
    bool isGameRunning = true;
    
    //Auto-repeater settings for leader auto-throws.
    AutoRepeaterSettings autoThrowSettings{
        LEADER::AUTO_THROW_SLOWEST_INTERVAL,
        LEADER::AUTO_THROW_FASTEST_INTERVAL,
        LEADER::AUTO_THROW_RAMP_TIME
    };
    
    //Buffer with the liquid limit effect.
    ALLEGRO_BITMAP* liquidLimitEffectBuffer = nullptr;
    
    //Info on every edge's liquid limit offset effect. Cache for performance.
    vector<EdgeOffsetCache> liquidLimitEffectCaches;
    
    //Loading screen subtext buffer.
    ALLEGRO_BITMAP* loadingSubtextBmp = nullptr;
    
    //Loading screen main text buffer.
    ALLEGRO_BITMAP* loadingTextBmp = nullptr;
    
    //List of mob actions.
    vector<MobAction> mobActions;
    
    //List of mob categories.
    CategoryManager mobCategories;
    
    //Current modal dialog, if any.
    ModalGuiManager modal;
    
    //Mouse cursor information.
    MouseCursor mouseCursor;
    
    //Database of all mission fail conditions.
    vector<MissionFail*> missionFailConds;
    
    //Database of all mission goals.
    vector<MissionGoal*> missionGoals;
    
    //Database of all mission score criteria.
    vector<MissionScoreCriterion*> missionScoreCriteria;
    
    //User options.
    Options options;
    
    //Performance monitor.
    PerformanceMonitor* perfMon = nullptr;
    
    //Player actions in this frame.
    vector<Inpution::Action> playerActions;
    
    //List of all possible inventory items.
    InventoryItemDatabase inventoryItems;
    
    //Randomness manager.
    RngManager rng;
    
    //Database of all sector types and their names.
    EnumNameDatabase sectorTypes;
    
    //Should we be showing system info? (Framerate, version, etc.)
    bool showSystemInfo = false;
    
    //Skip rendering the scene with Dear ImGui for this frame.
    //It's a bit of a hack that fixes some corner cases.
    bool skipDearImGuiFrame = false;
    
    //List of game states.
    GameStateList states;
    
    //List of lifetime statistics.
    Statistics statistics;
    
    //List of internal names of content that is needed by the system.
    SystemContentNames sysContentNames;
    
    //List of content that is needed system-wide.
    SystemContentList sysContent;
    
    //List of all mob team's internal names.
    string teamInternalNames[N_MOB_TEAMS];
    
    //List of all mob team names, in proper English.
    string teamNames[N_MOB_TEAMS];
    
    //How much time has passed since the program booted.
    float timePassed = 0.0f;
    
    //Viewport for the editor canvases.
    Viewport editorsView;
    
    //Buffer with the wall shadows and ledge smoothings.
    ALLEGRO_BITMAP* wallOffsetEffectBuffer = nullptr;
    
    //Info on every edge's wall shadow offset effect. Cache for performance.
    vector<EdgeOffsetCache> wallShadowEffectCaches;
    
    //Info on every edge's wall smoothing offset effect. Cache for performance.
    vector<EdgeOffsetCache> wallSmoothingEffectCaches;
    
    //Current fullscreen state.
    bool winFullscreen = OPTIONS::GRAPHICS_D::WIN_FULLSCREEN;
    
    //Current window height.
    unsigned int winH = OPTIONS::GRAPHICS_D::WIN_H;
    
    //Current window width.
    unsigned int winW = OPTIONS::GRAPHICS_D::WIN_W;
    
    //Data for when the quick play feature is used in an editor.
    struct {
    
        //--- Members ---
        
        //Path of the folder of the area to quick play.
        string areaPath;
        
        //Editor to return to from the area.
        GameState* editor = nullptr;
        
        //Path of the content that was being edited in the editor.
        string content;
        
        //Editor camera position before quick play.
        Point camPos;
        
        //Editor camera zoom before quick play.
        float camZ = 1.0f;
        
    } quickPlay;
    
    //Engine debugging tools.
    //Set them to true in the Game constructor as needed.
    struct {
    
        //--- Members ---
        
        //Print each frame's player actions to the terminal.
        bool printActions = false;
        
        //Show and operate on a Dear ImGui demo window.
        bool showDearImGuiDemo = false;
        
        //Show visual information about the leader's group.
        bool showGroupInfo = false;
        
        //Show which area cells are active in the radar.
        bool showAreaActiveCells = false;
        
    } debug;
    
    
    //--- Function declarations ---
    
    Game();
    void changeState(
        GameState* newState,
        bool unloadCurrent = true, bool loadNew = true
    );
    string getCurStateName() const;
    void unloadLoadedState(GameState* loadedState);
    void registerAudioStreamSource(ALLEGRO_AUDIO_STREAM* stream);
    void unregisterAudioStreamSource(ALLEGRO_AUDIO_STREAM* stream);
    int start();
    void mainLoop();
    void shutdown();
    
private:

    //--- Members ---
    
    //Current game state: title screen, gameplay, etc.
    GameState* curState = nullptr;
    
    //Queue of Allegro events.
    ALLEGRO_EVENT_QUEUE* eventQueue = nullptr;
    
    //Timer for the main frame logic.
    ALLEGRO_TIMER* mainTimer = nullptr;
    
    //Is deltaT meant to be reset for the next frame?
    bool resetDeltaT = true;
    
    
    //--- Function declarations ---
    
    void drawFramerateChart() const;
    void globalDrawing();
    void globalLogicPost();
    void globalLogicPre();
    void globalHandleAllegroEvent(const ALLEGRO_EVENT& ev);
    bool globalHandleSystemPlayerAction(const Inpution::Action& action);
    void processSystemInfo();
    
};


extern Game game;
