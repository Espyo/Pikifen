/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Global game-related functions.
 */

#include <algorithm>

#include <allegro5/allegro_native_dialog.h>

#include "game.h"

#include "../lib/imgui/imgui_impl_allegro5.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "drawing.h"
#include "init.h"
#include "load.h"
#include "misc_functions.h"


namespace GAME {

//Standard color of the mouse cursor.
const ALLEGRO_COLOR CURSOR_STANDARD_COLOR = al_map_rgb(188, 230, 230);

//Maximum alpha of the cursor's trail -- the alpha value near the cursor.
const unsigned char CURSOR_TRAIL_MAX_ALPHA = 72;

//Maximum width of the cursor's trail -- the width value near the cursor.
const float CURSOR_TRAIL_MAX_WIDTH = 30.0f;

//How far the cursor must move from its current spot before the next spot.
const float CURSOR_TRAIL_MIN_SPOT_DIFF = 4.0f;

//Every X seconds, the cursor's position is saved, to create the trail effect.
const float CURSOR_TRAIL_SAVE_INTERVAL = 0.016f;

//Number of positions of the cursor to keep track of.
const unsigned char CURSOR_TRAIL_SAVE_N_SPOTS = 16;

//Duration of full-window fades.
const float FADE_DURATION = 0.15f;

//Duration of slow full-window fades.
const float FADE_SLOW_DURATION = 0.5f;

//When getting a framerate average, use a sample of this size.
const size_t FRAMERATE_AVG_SAMPLE_SIZE = 30;

//Only save the latest N FPS samples.
const size_t FRAMERATE_HISTORY_SIZE = 300;

}


/**
 * @brief Constructs a new game class object.
 */
Game::Game() {
    teamInternalNames[MOB_TEAM_NONE] = "none";
    teamInternalNames[MOB_TEAM_PLAYER_1] = "player_1";
    teamInternalNames[MOB_TEAM_PLAYER_2] = "player_2";
    teamInternalNames[MOB_TEAM_PLAYER_3] = "player_3";
    teamInternalNames[MOB_TEAM_PLAYER_4] = "player_4";
    teamInternalNames[MOB_TEAM_ENEMY_1] = "enemy_1";
    teamInternalNames[MOB_TEAM_ENEMY_2] = "enemy_2";
    teamInternalNames[MOB_TEAM_ENEMY_3] = "enemy_3";
    teamInternalNames[MOB_TEAM_ENEMY_4] = "enemy_4";
    teamInternalNames[MOB_TEAM_OBSTACLE] = "obstacle";
    teamInternalNames[MOB_TEAM_OTHER] = "other";
    
    teamNames[MOB_TEAM_NONE] = "None";
    teamNames[MOB_TEAM_PLAYER_1] = "Player 1";
    teamNames[MOB_TEAM_PLAYER_2] = "Player 2";
    teamNames[MOB_TEAM_PLAYER_3] = "Player 3";
    teamNames[MOB_TEAM_PLAYER_4] = "Player 4";
    teamNames[MOB_TEAM_ENEMY_1] = "Enemy 1";
    teamNames[MOB_TEAM_ENEMY_2] = "Enemy 2";
    teamNames[MOB_TEAM_ENEMY_3] = "Enemy 3";
    teamNames[MOB_TEAM_ENEMY_4] = "Enemy 4";
    teamNames[MOB_TEAM_OBSTACLE] = "Obstacle";
    teamNames[MOB_TEAM_OTHER] = "Other";
}


/**
 * @brief Changes to a different game state.
 *
 * @param newState State to change to.
 * @param unloadCurrent If true, the current state is unloaded from memory.
 * @param loadNew If true, the new state is loaded to memory.
 * If you try to change to that state when it is not loaded,
 * things will go wrong.
 */
void Game::changeState(
    GameState* newState, bool unloadCurrent, bool loadNew
) {
    if(curState && unloadCurrent) {
        curState->unload();
        curState->loaded = false;
    }
    
    curState = newState;
    
    if(loadNew) {
        curState->load();
        curState->loaded = true;
    }
    
    //Because during the loading screens there is no activity, on the
    //next frame, the game will assume the time between that and the last
    //non-loading frame is normal. This could be something like 2 seconds.
    //Let's reset the deltaT, then.
    resetDeltaT = true;
}


/**
 * @brief Returns the name of the current state.
 *
 * @return The name.
 */
string Game::getCurStateName() const {
    if(!curState) return "none";
    return curState->getName();
}


/**
 * @brief Performs some global drawings to run every frame.
 */
void Game::globalDrawing() {
    //Dear ImGui.
    if(debug.showDearImGuiDemo) {
        ImGui::ShowDemoWindow();
    }
    ImGui::Render();
    if(!skipDearImGuiFrame) {
        ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());
    } else {
        skipDearImGuiFrame = false;
    }
    
    //Fade manager.
    if(!debug.showDearImGuiDemo) {
        game.fadeMgr.draw();
    }
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void Game::globalHandleAllegroEvent(const ALLEGRO_EVENT &ev) {
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        //Mouse cursor.
        mouseCursor.updatePos(ev);
        
    } else if(ev.type == ALLEGRO_EVENT_AUDIO_STREAM_FINISHED) {
        //Audio stream finished.
        game.audio.handleStreamFinished(
            (ALLEGRO_AUDIO_STREAM*) (ev.any.source)
        );
        
    } else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        //Hitting the X on the game window.
        isGameRunning = false;
        
    } else if(ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
        //On Windows, when you tab out then back in, sometimes you'd see
        //weird artifacts. This workaround fixes it.
        al_resize_display(display, winW, winH);
        
    }
    
    //Dear ImGui.
    ImGui_ImplAllegro5_ProcessEvent((ALLEGRO_EVENT*) &ev);
}


/**
 * @brief Handles a system player action, if possible.
 *
 * @param action The action.
 * @return Whether it got handled.
 */
bool Game::globalHandleSystemPlayerAction(const PlayerAction &action) {
    bool isSystemAction =
        controls.getPlayerActionType(action.actionTypeId).category ==
        PLAYER_ACTION_CAT_SYSTEM;
    if(!isSystemAction) return false;
    if(action.value < 0.5f) return false;
    
    switch(action.actionTypeId) {
    case PLAYER_ACTION_TYPE_SYSTEM_INFO: {
        showSystemInfo = !showSystemInfo;
        break;
        
    } case PLAYER_ACTION_TYPE_SCREENSHOT: {
        saveScreenshot();
        break;
    }
    }
    
    return true;
}


/**
 * @brief Performs some global logic to run every frame.
 */
void Game::globalLogic() {
    //Player action handling.
    for(size_t a = 0; a < playerActions.size();) {
        if(makerTools.handleGeneralPlayerAction(playerActions[a])) {
            playerActions.erase(playerActions.begin() + a);
        } else if(globalHandleSystemPlayerAction(playerActions[a])) {
            playerActions.erase(playerActions.begin() + a);
        } else {
            a++;
        }
    }
    
    //Cursor trail.
    if(options.advanced.drawCursorTrail) {
        mouseCursor.saveTimer.tick(deltaT);
    }
    
    //Audio.
    audio.tick(deltaT);
    
    //Dear ImGui.
    ImGui_ImplAllegro5_NewFrame();
    ImGui::NewFrame();
}


/**
 * @brief The main loop of the program. Processes events,
 * ticks frames of gameplay, etc.
 */
void Game::mainLoop() {
    //Used to calculate the time difference between the current and last frames.
    double prevFrameStartTime = 0.0;
    ALLEGRO_EVENT ev;
    
    //Main loop.
    al_start_timer(mainTimer);
    while(isGameRunning) {
    
        /*  ************************************************
          *** | _ |                                  | _ | ***
        *****  \_/           EVENT HANDLING           \_/  *****
          *** +---+                                  +---+ ***
            ************************************************/
        
        al_wait_for_event(eventQueue, &ev);
        
        globalHandleAllegroEvent(ev);
        curState->handleAllegroEvent(ev);
        controls.handleAllegroEvent(ev);
        
        switch(ev.type) {
        case ALLEGRO_EVENT_TIMER: {
            if(al_is_event_queue_empty(eventQueue)) {
            
                double curFrameStartTime = al_get_time();
                if(resetDeltaT) {
                    //Failsafe.
                    prevFrameStartTime =
                        curFrameStartTime -
                        1.0f / options.advanced.targetFps;
                    resetDeltaT = false;
                }
                
                float realDeltaT =
                    curFrameStartTime - prevFrameStartTime;
                statistics.runtime += realDeltaT;
                
                //Anti speed-burst cap.
                deltaT = std::min(realDeltaT, 0.2f);
                
                timePassed += deltaT;
                GameState* prevState = curState;
                
                playerActions = controls.newFrame(deltaT);
                globalLogic();
                curState->doLogic();
                
                if(curState == prevState) {
                    //Only draw if we didn't change states in the meantime.
                    curState->doDrawing();
                    globalDrawing();
                    al_flip_display();
                } else {
                    ImGui::EndFrame();
                }
                
                double curFrameEndTime = al_get_time();
                curFrameProcessTime =
                    curFrameEndTime - curFrameStartTime;
                    
                prevFrameStartTime = curFrameStartTime;
                
            }
            break;
            
        }
        }
    }
}


/**
 * @brief Registers an Allegro audio stream's event source into the event
 * queue.
 *
 * @param stream The audio stream.
 */
void Game::registerAudioStreamSource(ALLEGRO_AUDIO_STREAM* stream) {
    al_register_event_source(
        eventQueue,
        al_get_audio_stream_event_source(stream)
    );
}


/**
 * @brief Shuts down the program, cleanly freeing everything.
 */
void Game::shutdown() {
    if(perfMon) {
        perfMon->saveLog();
    }
    
    if(curState) {
        curState->unload();
    }
    
    content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MISC,
        CONTENT_TYPE_BITMAP,
        CONTENT_TYPE_SOUND,
        CONTENT_TYPE_SONG_TRACK,
        CONTENT_TYPE_SONG,
    }
    );
    
    delete dummyMobState;
    
    unloadMiscResources();
    destroyMobCategories();
    states.destroy();
    destroyMisc();
    destroyEventThings(mainTimer, eventQueue);
    destroyAllegro();
}


/**
 * @brief Starts up the program, setting up everything that's necessary.
 *
 * @return 0 if everything is okay, otherwise a return code to quit the
 * program with.
 */
int Game::start() {
    //Allegro initializations.
    initAllegro();
    
    //Panic check: is there a game_data folder?
    if(folderToVector(FOLDER_PATHS_FROM_ROOT::GAME_DATA, true).empty()) {
        string header =
            FOLDER_NAMES::GAME_DATA + " folder not found!";
        string text =
            "Could not find the \"" + FOLDER_NAMES::GAME_DATA + "\" folder! "
            "If you are running the engine from a zip file, "
            "you have to unpack it first.";
        showSystemMessageBox(
            nullptr, header.c_str(), header.c_str(), text.c_str(),
            nullptr, ALLEGRO_MESSAGEBOX_ERROR
        );
        return -1;
    }
    
    //Essentials.
    initEssentials();
    states.init();
    
    //Controls and options.
    initControls();
    loadOptions();
    saveOptions();
    loadStatistics();
    statistics.startups++;
    saveStatistics();
    
    //Event stuff.
    initEventThings(mainTimer, eventQueue);
    
    //Other fundamental initializations and loadings.
    initMisc();
    initErrorBitmap();
    content.reloadPacks();
    content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MISC,
        CONTENT_TYPE_BITMAP,
        CONTENT_TYPE_SOUND,
        CONTENT_TYPE_SONG_TRACK,
        CONTENT_TYPE_SONG,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    loadFonts();
    loadMiscGraphics();
    loadMiscSounds();
    
    //Draw the basic loading screen.
    drawLoadingScreen("", "", 1.0);
    al_flip_display();
    
    //Init Dear ImGui.
    initDearImGui();
    
    //Init and load some engine things.
    initMobActions();
    initMobCategories();
    initMiscDatabases();
    loadMakerTools();
    saveMakerTools();
    
    dummyMobState = new MobState("dummy");
    
    if(makerTools.usePerfMon) {
        perfMon = new PerformanceMonitor();
    }
    
    //Auto-start in some state.
    if(
        makerTools.enabled &&
        makerTools.autoStartState == "play" &&
        !makerTools.autoStartOption.empty()
    ) {
        states.gameplay->pathOfAreaToLoad =
            makerTools.autoStartOption;
        changeState(states.gameplay);
        
    } else if(
        makerTools.enabled &&
        makerTools.autoStartState == "animation_editor"
    ) {
        states.animationEd->autoLoadFile =
            makerTools.autoStartOption;
        changeState(states.animationEd);
        
    } else if(
        makerTools.enabled &&
        makerTools.autoStartState == "area_editor"
    ) {
        states.areaEd->autoLoadFolder =
            makerTools.autoStartOption;
        changeState(states.areaEd);
        
    } else if(
        makerTools.enabled &&
        makerTools.autoStartState == "gui_editor"
    ) {
        states.guiEd->autoLoadFile =
            makerTools.autoStartOption;
        changeState(states.guiEd);
    } else if(
        makerTools.enabled &&
        makerTools.autoStartState == "particle_editor"
    ) {
        states.particleEd->autoLoadFile =
            makerTools.autoStartOption;
        changeState(states.particleEd);
    } else {
        changeState(states.titleScreen);
    }
    
    return 0;
}


/**
 * @brief Unloads a loaded state that never got to be unloaded. This should only
 * be the case if changeState was called with instructions to not
 * unload the previous one.
 *
 * @param loadedState Loaded state to unload.
 */
void Game::unloadLoadedState(GameState* loadedState) {
    loadedState->unload();
}


/**
 * @brief Unregisters an Allegro audio stream's event source from the event
 * queue.
 *
 * @param stream The audio stream.
 */
void Game::unregisterAudioStreamSource(ALLEGRO_AUDIO_STREAM* stream) {
    al_unregister_event_source(
        eventQueue,
        al_get_audio_stream_event_source(stream)
    );
}


/**
 * @brief Destroys the states in the list.
 */
void GameStateList::destroy() {
    delete animationEd;
    delete areaEd;
    delete annexScreen;
    delete gameplay;
    delete guiEd;
    delete particleEd;
    delete titleScreen;
    delete results;
    
    animationEd = nullptr;
    areaEd = nullptr;
    gameplay = nullptr;
    guiEd = nullptr;
    particleEd = nullptr;
    titleScreen = nullptr;
    results = nullptr;
}


/**
 * @brief Initializes the states in the list.
 */
void GameStateList::init() {
    animationEd = new AnimationEditor();
    areaEd = new AreaEditor();
    annexScreen = new AnnexScreen();
    gameplay = new GameplayState();
    guiEd = new GuiEditor();
    particleEd = new ParticleEditor();
    titleScreen = new TitleScreen();
    results = new Results();
}


Game game;
