/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gameplay state class and
 * gameplay state-related functions.
 */

#include <algorithm>

#include <allegro5/allegro_native_dialog.h>

#include "gameplay.h"

#include "../../content/mob/converter.h"
#include "../../content/mob/pile.h"
#include "../../content/mob/resource.h"
#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../core/misc_structs.h"
#include "../../lib/data_file/data_file.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


namespace GAMEPLAY {

//How long the HUD moves for when the area is entered.
const float AREA_INTRO_HUD_MOVE_TIME = 3.0f;

//How long it takes for the area name to fade away, in-game.
const float AREA_TITLE_FADE_DURATION = 1.0f;

//How long the "Go!" big message lasts for.
const float BIG_MSG_GO_DUR = 1.5f;

//What text to show in the "Go!" big message.
const string BIG_MSG_GO_TEXT = "GO!";

//How long the "Mission clear!" big message lasts for.
const float BIG_MSG_MISSION_CLEAR_DUR = 4.5f;

//What text to show in the "Mission clear!" big message.
const string BIG_MSG_MISSION_CLEAR_TEXT = "MISSION CLEAR!";

//How long the "Mission failed..." big message lasts for.
const float BIG_MSG_MISSION_FAILED_DUR = 4.5f;

//What text to show in the "Mission failed..." big message.
const string BIG_MSG_MISSION_FAILED_TEXT = "MISSION FAILED...";

//How long the "Ready?" big message lasts for.
const float BIG_MSG_READY_DUR = 2.5f;

//What text to show in the "Ready?" big message.
const string BIG_MSG_READY_TEXT = "READY?";

//Distance between current leader and boss before the boss music kicks in.
const float BOSS_MUSIC_DISTANCE = 300.0f;

//Something is only considered off-camera if it's beyond this extra margin.
const float CAMERA_BOX_MARGIN = 128.0f;

//Dampen the camera's movements by this much.
const float CAMERA_SMOOTHNESS_MULT = 4.5f;

//Opacity of the collision bubbles in the maker tool.
const unsigned char COLLISION_OPACITY = 192;

//If an enemy is this close to the active leader, turn on the song's enemy mix.
const float ENEMY_MIX_DISTANCE = 150.0f;

//Width and height of the fog bitmap.
const int FOG_BITMAP_SIZE = 128;

//How long the HUD moves for when a menu is entered.
const float MENU_ENTRY_HUD_MOVE_TIME = 0.4f;

//How long the HUD moves for when a menu is exited.
const float MENU_EXIT_HUD_MOVE_TIME = 0.5f;

//When a leader lands, this is the maximum size of the particles.
extern const float LEADER_LAND_PART_MAX_SIZE = 64.0f;

//When a leader lands, scale the particles by the fall distance and this factor.
extern const float LEADER_LAND_PART_SIZE_MULT = 0.1f;

//Opacity of the throw preview.
const unsigned char PREVIEW_OPACITY = 160;

//Scale of the throw preview's effect texture.
const float PREVIEW_TEXTURE_SCALE = 20.0f;

//Time multiplier for the throw preview's effect texture animation.
const float PREVIEW_TEXTURE_TIME_MULT = 20.0f;

//How frequently should a replay state be saved.
const float REPLAY_SAVE_FREQUENCY = 1.0f;

//Swarming arrows move these many units per second.
const float SWARM_ARROW_SPEED = 400.0f;

//Tree shadows sway this much away from their neutral position.
const float TREE_SHADOW_SWAY_AMOUNT = 8.0f;

//Tree shadows sway this much per second (TAU = full back-and-forth cycle).
const float TREE_SHADOW_SWAY_SPEED = TAU / 8;

}


/**
 * @brief Changes the amount of sprays of a certain type the player owns.
 * It also animates the correct HUD item, if any.
 *
 * @param team Which team's spray counts to change.
 * @param typeIdx Index number of the spray type.
 * @param amount Amount to change by.
 */
void GameplayState::changeSprayCount(
    PlayerTeam* team, size_t typeIdx, signed int amount
) {
    team->sprayStats[typeIdx].nrSprays =
        std::max(
            (signed int) team->sprayStats[typeIdx].nrSprays + amount,
            (signed int) 0
        );
        
    for(Player* player : team->players) {
        GuiItem* sprayHudItem = nullptr;
        if(game.content.sprayTypes.list.size() > 2) {
            if(player->selectedSpray == typeIdx) {
                sprayHudItem = player->hud->spray1Amount;
            }
        } else {
            if(typeIdx == 0) {
                sprayHudItem = player->hud->spray1Amount;
            } else {
                sprayHudItem = player->hud->spray2Amount;
            }
        }
        if(sprayHudItem) {
            sprayHudItem->startJuiceAnimation(
                GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
        }
    }
}


/**
 * @brief Draws the gameplay.
 */
void GameplayState::doDrawing() {
    doGameDrawing();
    
    if(game.perfMon) {
        game.perfMon->leaveState();
    }
}


/**
 * @brief Tick the gameplay logic by one frame.
 */
void GameplayState::doLogic() {
    if(game.perfMon) {
        if(isInputAllowed) {
            //The first frame will have its speed all broken,
            //because of the long loading time that came before it.
            game.perfMon->setPaused(false);
            game.perfMon->enterState(PERF_MON_STATE_FRAME);
        } else {
            game.perfMon->setPaused(true);
        }
    }
    
    float regularDeltaT = game.deltaT;
    
    if(game.makerTools.changeSpeed) {
        game.deltaT *=
            game.makerTools.changeSpeedSettings[
                game.makerTools.changeSpeedSettingIdx
            ];
    }
    
    for(Player &player : players) {
        player.view.updateCursor(game.mouseCursor.winPos);
    }
    
    //Controls.
    for(size_t a = 0; a < game.playerActions.size(); a++) {
        handlePlayerAction(game.playerActions[a]);
        if(onionMenu) onionMenu->handlePlayerAction(game.playerActions[a]);
        if(pauseMenu) pauseMenu->handlePlayerAction(game.playerActions[a]);
        game.makerTools.handleGameplayPlayerAction(game.playerActions[a]);
    }
    
    //Game logic.
    if(!paused) {
        game.statistics.gameplayTime += regularDeltaT;
        doGameplayLogic(game.deltaT * deltaTMult);
        doAestheticLogic(game.deltaT * deltaTMult);
    }
    doMenuLogic();
}


/**
 * @brief Ends the currently ongoing mission.
 *
 * @param cleared Did the player reach the goal?
 */
void GameplayState::endMission(bool cleared) {
    if(curInterlude != INTERLUDE_NONE) {
        return;
    }
    curInterlude = INTERLUDE_MISSION_END;
    interludeTime = 0.0f;
    deltaTMult = 0.5f;
    for(Player &player : players) {
        player.leaderMovement.reset(); //TODO replace with a better solution.
    }
    
    //Zoom in on the reason, if possible.
    for(Player &player : players) {
        Point newCamPos = player.view.cam.targetPos;
        float newCamZoom = player.view.cam.targetZoom;
        if(cleared) {
            MissionGoal* goal =
                game.missionGoals[game.curAreaData->mission.goal];
            if(goal->getEndZoomData(this, &newCamPos, &newCamZoom)) {
                player.view.cam.targetPos = newCamPos;
                player.view.cam.targetZoom = newCamZoom;
            }
            
        } else {
            MissionFail* cond =
                game.missionFailConds[missionFailReason];
            if(cond->getEndZoomData(this, &newCamPos, &newCamZoom)) {
                player.view.cam.targetPos = newCamPos;
                player.view.cam.targetZoom = newCamZoom;
            }
        }
    }
    
    if(cleared) {
        curBigMsg = BIG_MESSAGE_MISSION_CLEAR;
    } else {
        curBigMsg = BIG_MESSAGE_MISSION_FAILED;
    }
    bigMsgTime = 0.0f;
    
    for(Player &player : players) {
        player.hud->gui.startAnimation(
            GUI_MANAGER_ANIM_IN_TO_OUT,
            GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
        );
    }
}


/**
 * @brief Code to run when the state is entered, be it from the area menu, be it
 * from the result menu's "keep playing" option.
 */
void GameplayState::enter() {
    particles.viewports.clear();
    
    for(Player &player : players) {
        player.view.size.x = game.winW;
        player.view.size.y = game.winH;
        player.view.center.x = game.winW / 2.0f;
        player.view.center.y = game.winH / 2.0f;
        player.view.boxMargin.x = GAMEPLAY::CAMERA_BOX_MARGIN;
        player.view.boxMargin.y = GAMEPLAY::CAMERA_BOX_MARGIN;
        player.view.updateTransformations();
    }
    
    float zoomReaches[3] = {
        game.config.rules.zoomClosestReach,
        game.options.advanced.zoomMediumReach,
        game.config.rules.zoomFarthestReach
    };
    float viewportReach = sqrt(players[0].view.size.x * players[0].view.size.y);
    for(int z = 0; z < 3; z++) {
        zoomLevels[z] = viewportReach / zoomReaches[z];
    }
    
    for(Player &player : players) {
        if(player.leaderPtr) {
            player.view.cam.setPos(player.leaderPtr->pos);
        } else {
            player.view.cam.setPos(Point());
        }
        player.view.cam.setZoom(zoomLevels[1]);
        player.view.updateTransformations();
        particles.viewports.push_back(&player.view);
    }
    
    lastEnemyDefeatedPos = Point(LARGE_FLOAT);
    lastHurtLeaderPos = Point(LARGE_FLOAT);
    lastPikminBornPos = Point(LARGE_FLOAT);
    lastPikminDeathPos = Point(LARGE_FLOAT);
    lastShipThatGotTreasurePos = Point(LARGE_FLOAT);
    
    missionFailReason = (MISSION_FAIL_COND) INVALID;
    goalIndicatorRatio = 0.0f;
    fail1IndicatorRatio = 0.0f;
    fail2IndicatorRatio = 0.0f;
    scoreIndicator = 0.0f;
    
    paused = false;
    curInterlude = INTERLUDE_READY;
    interludeTime = 0.0f;
    curBigMsg = BIG_MESSAGE_READY;
    bigMsgTime = 0.0f;
    deltaTMult = 0.5f;
    bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    
    if(!game.states.areaEd->quickPlayAreaPath.empty()) {
        //If this is an area editor quick play, skip the "Ready..." interlude.
        interludeTime = GAMEPLAY::BIG_MSG_READY_DUR;
        bigMsgTime = GAMEPLAY::BIG_MSG_READY_DUR;
    }
    
    if(wentToResults) {
        game.fadeMgr.startFade(true, nullptr);
        if(pauseMenu) {
            pauseMenu->toDelete = true;
        }
    }
    
    readyForInput = false;
    game.mouseCursor.reset();
    
    for(Player &player : players) {
        player.hud->gui.hideItems();
        player.notification.reset();
        player.leaderCursorWorld = player.view.cursorWorldPos;
        player.leaderCursorWin = game.mouseCursor.winPos;
        if(player.leaderPtr) {
            player.leaderPtr->stopWhistling();
        }
        updateClosestGroupMembers(&player);
        
        player.whistle.nextDotTimer.onEnd = [&player] () {
            player.whistle.nextDotTimer.start();
            unsigned char dot = 255;
            for(unsigned char d = 0; d < 6; d++) { //Find WHAT dot to add.
                if(player.whistle.dotRadius[d] == -1) {
                    dot = d;
                    break;
                }
            }
            
            if(dot != 255) player.whistle.dotRadius[dot] = 0;
        };
        
        player.whistle.nextRingTimer.onEnd = [&player] () {
            player.whistle.nextRingTimer.start();
            player.whistle.rings.push_back(0);
            player.whistle.ringColors.push_back(player.whistle.ringPrevColor);
            player.whistle.ringPrevColor =
                sumAndWrap(player.whistle.ringPrevColor, 1, WHISTLE::N_RING_COLORS);
        };
    }
}


/**
 * @brief Generates the bitmap that'll draw the fog fade effect.
 *
 * @param nearRadius Until this radius, the fog is not present.
 * @param farRadius From this radius on, the fog is fully dense.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* GameplayState::generateFogBitmap(
    float nearRadius, float farRadius
) {
    if(farRadius == 0) return nullptr;
    
    ALLEGRO_BITMAP* bmp =
        al_create_bitmap(GAMEPLAY::FOG_BITMAP_SIZE, GAMEPLAY::FOG_BITMAP_SIZE);
        
    ALLEGRO_LOCKED_REGION* region =
        al_lock_bitmap(
            bmp, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_WRITEONLY
        );
    unsigned char* row = (unsigned char*) region->data;
    
    //We need to draw a radial gradient to represent the fog.
    //Between the center and the "near" radius, the opacity is 0%.
    //From there to the edge, the opacity fades to 100%.
    //Because the every quadrant of the image is the same, just mirrored,
    //we only need to process the pixels on the top-left quadrant and then
    //apply them to the respective pixels on the other quadrants as well.
    
    //This is where the "near" section of the fog is.
    float nearRatio = nearRadius / farRadius;
    
#define fillPixel(x, row) \
    row[(x) * 4 + 0] = 255; \
    row[(x) * 4 + 1] = 255; \
    row[(x) * 4 + 2] = 255; \
    row[(x) * 4 + 3] = curA; \
    
    for(int y = 0; y < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); y++) {
        for(int x = 0; x < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); x++) {
            //First, get how far this pixel is from the center.
            //Center = 0, radius or beyond = 1.
            float curRatio =
                Distance(
                    Point(x, y),
                    Point(
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0,
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0
                    )
                ).toFloat() / (GAMEPLAY::FOG_BITMAP_SIZE / 2.0);
            curRatio = std::min(curRatio, 1.0f);
            //Then, map that ratio to a different ratio that considers
            //the start of the "near" section as 0.
            curRatio =
                interpolateNumber(curRatio, nearRatio, 1.0f, 0.0f, 1.0f);
            //Finally, clamp the value and get the alpha.
            curRatio = std::clamp(curRatio, 0.0f, 1.0f);
            unsigned char curA = 255 * curRatio;
            
            //Save the memory location of the opposite row's pixels.
            unsigned char* oppositeRow =
                row + region->pitch * (GAMEPLAY::FOG_BITMAP_SIZE - y - y - 1);
            fillPixel(x, row);
            fillPixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, row);
            fillPixel(x, oppositeRow);
            fillPixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, oppositeRow);
        }
        row += region->pitch;
    }
    
#undef fillPixel
    
    al_unlock_bitmap(bmp);
    bmp = recreateBitmap(bmp); //Refresh mipmaps.
    return bmp;
}


/**
 * @brief Returns how many Pikmin are in the field in the current area.
 * This also checks inside converters.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t GameplayState::getAmountOfFieldPikmin(const PikminType* filter) {
    size_t total = 0;
    
    //Check the Pikmin mobs.
    for(size_t p = 0; p < mobs.pikmin.size(); p++) {
        Pikmin* pPtr = mobs.pikmin[p];
        if(filter && pPtr->pikType != filter) continue;
        total++;
    }
    
    //Check Pikmin inside converters.
    for(size_t c = 0; c < mobs.converters.size(); c++) {
        Converter* cPtr = mobs.converters[c];
        if(filter && cPtr->currentType != filter) continue;
        total += cPtr->amountInBuffer;
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are in the group.
 *
 * @param player The player responsible.
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t GameplayState::getAmountOfGroupPikmin(
    Player* player, const PikminType* filter
) {
    if(!player->leaderPtr) return 0;
    
    size_t total = 0;
    
    for(size_t m = 0; m < player->leaderPtr->group->members.size(); m++) {
        Mob* mPtr = player->leaderPtr->group->members[m];
        if(mPtr->type->category->id != MOB_CATEGORY_PIKMIN) continue;
        if(filter && mPtr->type != filter) continue;
        total++;
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are idling in the area.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t GameplayState::getAmountOfIdlePikmin(const PikminType* filter) {
    size_t total = 0;
    
    for(size_t p = 0; p < mobs.pikmin.size(); p++) {
        Pikmin* pPtr = mobs.pikmin[p];
        if(filter && pPtr->type != filter) continue;
        if(
            pPtr->fsm.curState->id == PIKMIN_STATE_IDLING ||
            pPtr->fsm.curState->id == PIKMIN_STATE_IDLING_H
        ) {
            total++;
        }
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are inside of Onions in the current area.
 * This also checks ships.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
long GameplayState::getAmountOfOnionPikmin(const PikminType* filter) {
    long total = 0;
    
    //Check Onions proper.
    for(size_t o = 0; o < mobs.onions.size(); o++) {
        Onion* oPtr = mobs.onions[o];
        for(
            size_t t = 0;
            t < oPtr->oniType->nest->pikTypes.size();
            t++
        ) {
            if(filter && oPtr->oniType->nest->pikTypes[t] != filter) {
                continue;
            }
            for(size_t m = 0; m < N_MATURITIES; m++) {
                total += (long) oPtr->nest->pikminInside[t][m];
            }
        }
    }
    
    //Check ships.
    for(size_t s = 0; s < mobs.ships.size(); s++) {
        Ship* sPtr = mobs.ships[s];
        if(!sPtr->nest) continue;
        for(
            size_t t = 0;
            t < sPtr->shiType->nest->pikTypes.size();
            t++
        ) {
            if(filter && sPtr->shiType->nest->pikTypes[t] != filter) {
                continue;
            }
            for(size_t m = 0; m < N_MATURITIES; m++) {
                total += (long) sPtr->nest->pikminInside[t][m];
            }
        }
    }
    return total;
}


/**
 * @brief Returns the total amount of Pikmin the player has.
 * This includes Pikmin in the field as well as the Onions, and also
 * Pikmin inside converters.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
long GameplayState::getAmountOfTotalPikmin(const PikminType* filter) {
    long total = 0;
    
    //Check Pikmin in the field and inside converters.
    total += (long) getAmountOfFieldPikmin(filter);
    
    //Check Pikmin inside Onions and ships.
    total += getAmountOfOnionPikmin(filter);
    
    //Return the final sum.
    return total;
}


/**
 * @brief Returns the closest group member of a given standby subgroup.
 * In the case all candidate members are out of reach,
 * this returns the closest. Otherwise, it returns the closest
 * and more mature one.
 *
 * @param player The player responsible.
 * @param type Type to search for.
 * @param distant If not nullptr, whether all members are unreachable is
 * returned here.
 * @return The closest member, or nullptr if there is no member
 * of that subgroup available to grab.
 */
Mob* GameplayState::getClosestGroupMember(
    Player* player, const SubgroupType* type, bool* distant
) {
    if(!player->leaderPtr) return nullptr;
    
    Mob* result = nullptr;
    
    //Closest members so far for each maturity.
    Distance closestDists[N_MATURITIES];
    Mob* closestPtrs[N_MATURITIES];
    bool canGrabClosest[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; m++) {
        closestPtrs[m] = nullptr;
        canGrabClosest[m] = false;
    }
    
    //Fetch the closest, for each maturity.
    size_t nMembers = player->leaderPtr->group->members.size();
    for(size_t m = 0; m < nMembers; m++) {
    
        Mob* memberPtr = player->leaderPtr->group->members[m];
        if(memberPtr->subgroupTypePtr != type) {
            continue;
        }
        
        unsigned char maturity = 0;
        if(memberPtr->type->category->id == MOB_CATEGORY_PIKMIN) {
            maturity = ((Pikmin*) memberPtr)->maturity;
        }
        bool canGrab = player->leaderPtr->canGrabGroupMember(memberPtr);
        
        if(!canGrab && canGrabClosest[maturity]) {
            //Skip if we'd replace a grabbable Pikmin with a non-grabbable one.
            continue;
        }
        
        Distance d(player->leaderPtr->pos, memberPtr->pos);
        
        if(
            (canGrab && !canGrabClosest[maturity]) ||
            !closestPtrs[maturity] ||
            d < closestDists[maturity]
        ) {
            closestDists[maturity] = d;
            closestPtrs[maturity] = memberPtr;
            canGrabClosest[maturity] = canGrab;
            
        }
    }
    
    //Now, try to get the one with the highest maturity within reach.
    Distance closestDist;
    for(unsigned char m = 0; m < N_MATURITIES; m++) {
        if(!closestPtrs[2 - m]) continue;
        if(!canGrabClosest[2 - m]) continue;
        result = closestPtrs[2 - m];
        closestDist = closestDists[2 - m];
        break;
    }
    
    if(distant) *distant = !result;
    
    if(!result) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; m++) {
            if(!closestPtrs[m]) continue;
            
            if(!result || closestDists[m] < closestDist) {
                result = closestPtrs[m];
                closestDist = closestDists[m];
            }
        }
    }
    
    return result;
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string GameplayState::getName() const {
    return "gameplay";
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev Event to handle.
 */
void GameplayState::handleAllegroEvent(ALLEGRO_EVENT &ev) {
    //Handle the Onion menu first so events don't bleed from gameplay to it.
    if(onionMenu) {
        onionMenu->handleAllegroEvent(ev);
    } else if(pauseMenu) {
        pauseMenu->handleAllegroEvent(ev);
    }
    
    if(ev.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
        game.controls.releaseAll();
    }
    
    //Finally, let the HUD handle events.
    for(Player &player : players) {
        player.hud->gui.handleAllegroEvent(ev);
    }
    
}


/**
 * @brief Leaves the gameplay state and enters the title screen,
 * or annex screen, or etc.
 *
 * @param target Where to leave to.
 */
void GameplayState::leave(const GAMEPLAY_LEAVE_TARGET target) {
    if(unloading) return;
    
    if(game.perfMon) {
        //Don't register the final frame, since it won't draw anything.
        game.perfMon->setPaused(true);
    }
    
    game.audio.stopAllPlaybacks();
    game.audio.setCurrentSong("");
    bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    saveStatistics();
    
    switch(target) {
    case GAMEPLAY_LEAVE_TARGET_RETRY: {
        game.changeState(game.states.gameplay);
        break;
    } case GAMEPLAY_LEAVE_TARGET_END: {
        wentToResults = true;
        //Change state, but don't unload this one, since the player
        //may pick the "keep playing" option in the results screen.
        game.changeState(game.states.results, false);
        break;
    } case GAMEPLAY_LEAVE_TARGET_AREA_SELECT: {
        if(game.states.areaEd->quickPlayAreaPath.empty()) {
            game.states.annexScreen->areaMenuAreaType =
                game.curAreaData->type;
            game.states.annexScreen->menuToLoad =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.changeState(game.states.annexScreen);
        } else {
            game.changeState(game.states.areaEd);
        }
        break;
    }
    }
}


/**
 * @brief Loads the "gameplay" state into memory.
 */
void GameplayState::load() {
    if(game.perfMon) {
        game.perfMon->reset();
        game.perfMon->enterState(PERF_MON_STATE_LOADING);
        game.perfMon->setPaused(false);
    }
    
    loading = true;
    game.errors.prepareAreaLoad();
    wentToResults = false;
    
    drawLoadingScreen("", "", 1.0f);
    al_flip_display();
    
    game.statistics.areaEntries++;
    
    //Game content.
    loadGameContent();
    
    //Initialize some important things.
    for(size_t t = 0; t < MAX_PLAYER_TEAMS; t++) {
        for(size_t s = 0; s < game.content.sprayTypes.list.size(); s++) {
            playerTeams[t].sprayStats.push_back(SprayStats());
        }
    }
    players[0].team = &playerTeams[0];
    
    areaTimePassed = 0.0f;
    gameplayTimePassed = 0.0f;
    game.makerTools.resetForGameplay();
    areaTitleFadeTimer.start();
    
    afterHours = false;
    pikminBorn = 0;
    pikminDeaths = 0;
    treasuresCollected = 0;
    treasuresTotal = 0;
    goalTreasuresCollected = 0;
    goalTreasuresTotal = 0;
    treasurePointsCollected = 0;
    treasurePointsTotal = 0;
    enemyDefeats = 0;
    enemyTotal = 0;
    enemyPointsCollected = 0;
    enemyPointsTotal = 0;
    curLeadersInMissionExit = 0;
    missionRequiredMobAmount = 0;
    missionScore = 0;
    oldMissionScore = 0;
    oldMissionGoalCur = 0;
    oldMissionFail1Cur = 0;
    oldMissionFail2Cur = 0;
    nrLivingLeaders = 0;
    leadersKod = 0;
    
    game.framerateLastAvgPoint = 0;
    game.framerateHistory.clear();
    
    bossMusicState = BOSS_MUSIC_STATE_NEVER_PLAYED;
    game.audio.setCurrentSong("");
    game.audio.onSongFinished = [this] (const string &name) {
        if(name == game.sysContentNames.sngBossVictory) {
            switch(bossMusicState) {
            case BOSS_MUSIC_STATE_VICTORY: {
                game.audio.setCurrentSong(game.curAreaData->songName, false);
                bossMusicState = BOSS_MUSIC_STATE_PAUSED;
            } default: {
                break;
            }
            }
        }
    };
    
    auto sparkAnimDbIt =
        game.content.globalAnimDbs.list.find(
            game.sysContentNames.anmSparks
        );
    if(sparkAnimDbIt == game.content.globalAnimDbs.list.end()) {
        game.errors.report(
            "Unknown global animation \"" + game.sysContentNames.anmSparks +
            "\" when trying to load the leader damage sparks!"
        );
    } else {
        game.sysContent.anmSparks.initToFirstAnim(
            &sparkAnimDbIt->second
        );
    }
    
    //Load the area.
    if(
        !game.content.loadAreaAsCurrent(
            pathOfAreaToLoad, nullptr,
            CONTENT_LOAD_LEVEL_FULL, false
        )
    ) {
        game.changeState(game.states.titleScreen, true);
        return;
    }
    
    if(!game.curAreaData->weatherCondition.blackoutStrength.empty()) {
        lightmapBmp = al_create_bitmap(game.winW, game.winH);
    }
    if(!game.curAreaData->weatherCondition.fogColor.empty()) {
        bmpFog =
            generateFogBitmap(
                game.curAreaData->weatherCondition.fogNear,
                game.curAreaData->weatherCondition.fogFar
            );
    }
    
    //Generate mobs.
    nextMobId = 0;
    if(game.perfMon) {
        game.perfMon->startMeasurement("Object generation");
    }
    
    vector<Mob*> mobsPerGen;
    
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* mPtr = game.curAreaData->mobGenerators[m];
        bool valid = true;
        
        if(!mPtr->type) {
            valid = false;
        } else if(
            mPtr->type->category->id == MOB_CATEGORY_PIKMIN &&
            game.states.gameplay->mobs.pikmin.size() >=
            game.config.rules.maxPikminInField
        ) {
            valid = false;
        }
        
        if(valid) {
            Mob* newMob =
                createMob(
                    mPtr->type->category, mPtr->pos, mPtr->type,
                    mPtr->angle, mPtr->vars
                );
            mobsPerGen.push_back(newMob);
        } else {
            mobsPerGen.push_back(nullptr);
        }
    }
    
    //Mob links.
    //Because mobs can create other mobs when loaded, mob gen index X
    //does not necessarily correspond to mob index X. Hence, we need
    //to keep the pointers to the created mobs in a vector, and use this
    //to link the mobs by (generator) index.
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* genPtr = game.curAreaData->mobGenerators[m];
        Mob* mobPtr = mobsPerGen[m];
        if(!mobPtr) continue;
        
        for(size_t l = 0; l < genPtr->linkIdxs.size(); l++) {
            size_t linkTargetGenIdx = genPtr->linkIdxs[l];
            Mob* linkTargetMobPtr = mobsPerGen[linkTargetGenIdx];
            mobPtr->push_anonymous_link(linkTargetMobPtr);
        }
    }
    
    //Mobs stored inside other. Same logic as mob links.
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* holdeeGenPtr = game.curAreaData->mobGenerators[m];
        if(holdeeGenPtr->storedInside == INVALID) continue;
        Mob* holdeePtr = mobsPerGen[m];
        Mob* holderMobPtr = mobsPerGen[holdeeGenPtr->storedInside];
        holderMobPtr->storeMobInside(holdeePtr);
    }
    
    //Save each path stop's sector.
    for(size_t s = 0; s < game.curAreaData->pathStops.size(); s++) {
        game.curAreaData->pathStops[s]->sectorPtr =
            getSector(game.curAreaData->pathStops[s]->pos, nullptr, true);
    }
    
    //Sort leaders.
    sort(
        mobs.leaders.begin(), mobs.leaders.end(),
    [] (Leader * l1, Leader * l2) -> bool {
        size_t priorityL1 =
        find(
            game.config.leaders.order.begin(),
            game.config.leaders.order.end(), l1->leaType
        ) -
        game.config.leaders.order.begin();
        size_t priorityL2 =
        find(
            game.config.leaders.order.begin(),
            game.config.leaders.order.end(), l2->leaType
        ) -
        game.config.leaders.order.begin();
        return priorityL1 < priorityL2;
    }
    );
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //In case a leader is stored in another mob,
    //update the available list.
    updateAvailableLeaders();
    startingNrOfLeaders = mobs.leaders.size();
    
    for(Player &player : players) {
        player.leaderIdx = INVALID;
        player.leaderPtr = nullptr;
        
        if(!mobs.leaders.empty()) {
            changeToNextLeader(&player, true, false, false);
        }
        
        player.whistle.nextDotTimer.start();
        player.whistle.nextRingTimer.start();
    }
    
    //Memorize mobs required by the mission.
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        unordered_set<size_t> missionRequiredMobGenIdxs;
        
        if(game.curAreaData->mission.goalAllMobs) {
            for(size_t m = 0; m < mobsPerGen.size(); m++) {
                if(
                    mobsPerGen[m] &&
                    game.missionGoals[game.curAreaData->mission.goal]->
                    isMobApplicable(mobsPerGen[m]->type)
                ) {
                    missionRequiredMobGenIdxs.insert(m);
                }
            }
            
        } else {
            missionRequiredMobGenIdxs =
                game.curAreaData->mission.goalMobIdxs;
        }
        
        for(size_t i : missionRequiredMobGenIdxs) {
            missionRemainingMobIds.insert(mobsPerGen[i]->id);
        }
        missionRequiredMobAmount = missionRemainingMobIds.size();
        
        if(game.curAreaData->mission.goal == MISSION_GOAL_COLLECT_TREASURE) {
            //Since the collect treasure goal can accept piles and resources
            //meant to add treasure points, we'll need some special treatment.
            for(size_t i : missionRequiredMobGenIdxs) {
                if(
                    mobsPerGen[i]->type->category->id ==
                    MOB_CATEGORY_PILES
                ) {
                    Pile* pilPtr = (Pile*) mobsPerGen[i];
                    goalTreasuresTotal += pilPtr->amount;
                } else {
                    goalTreasuresTotal++;
                }
            }
        }
    }
    
    //Figure out the total amount of treasures and their points.
    for(size_t t = 0; t < mobs.treasures.size(); t++) {
        treasuresTotal++;
        treasurePointsTotal +=
            mobs.treasures[t]->treType->points;
    }
    for(size_t p = 0; p < mobs.piles.size(); p++) {
        Pile* pPtr = mobs.piles[p];
        ResourceType* resType = pPtr->pilType->contents;
        if(
            resType->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasuresTotal += pPtr->amount;
        treasurePointsTotal +=
            pPtr->amount * resType->pointAmount;
    }
    for(size_t r = 0; r < mobs.resources.size(); r++) {
        Resource* rPtr = mobs.resources[r];
        if(
            rPtr->resType->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasuresTotal++;
        treasurePointsTotal += rPtr->resType->pointAmount;
    }
    
    //Figure out the total amount of enemies and their points.
    enemyTotal = mobs.enemies.size();
    for(size_t e = 0; e < mobs.enemies.size(); e++) {
        enemyPointsTotal += mobs.enemies[e]->eneType->points;
    }
    
    //Initialize the area's active cells.
    float areaWidth =
        game.curAreaData->bmap.nCols * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    float areaHeight =
        game.curAreaData->bmap.nRows * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    size_t nrAreaCellCols =
        ceil(areaWidth / GEOMETRY::AREA_CELL_SIZE) + 1;
    size_t nrAreaCellRows =
        ceil(areaHeight / GEOMETRY::AREA_CELL_SIZE) + 1;
        
    areaActiveCells.clear();
    areaActiveCells.assign(
        nrAreaCellCols, vector<bool>(nrAreaCellRows, false)
    );
    
    //Initialize some other things.
    pathMgr.handleAreaLoad();
    
    for(Player &player : players) {
        player.hud = new Hud();
        player.hud->player = &player;
    }
    
    dayMinutes = game.curAreaData->dayTimeStart;
    
    map<string, string> sprayStrs =
        getVarMap(game.curAreaData->sprayAmounts);
        
    for(auto &s : sprayStrs) {
        size_t sprayIdx = 0;
        for(; sprayIdx < game.config.misc.sprayOrder.size(); sprayIdx++) {
            if(game.config.misc.sprayOrder[sprayIdx]->manifest->internalName == s.first) {
                break;
            }
        }
        if(sprayIdx == game.content.sprayTypes.list.size()) {
            game.errors.report(
                "Unknown spray type \"" + s.first + "\", "
                "while trying to set the starting number of sprays for "
                "area \"" + game.curAreaData->name + "\"!", nullptr
            );
            continue;
        }
        
        for(size_t t = 0; t < MAX_PLAYER_TEAMS; t++) {
            playerTeams[t].sprayStats[sprayIdx].nrSprays = s2i(s.second);
        }
    }
    
    //Effect caches.
    game.liquidLimitEffectCaches.clear();
    game.liquidLimitEffectCaches.insert(
        game.liquidLimitEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.liquidLimitEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveLiquidLimit,
        getLiquidLimitLength,
        getLiquidLimitColor
    );
    game.wallSmoothingEffectCaches.clear();
    game.wallSmoothingEffectCaches.insert(
        game.wallSmoothingEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.wallSmoothingEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveLedgeSmoothing,
        getLedgeSmoothingLength,
        getLedgeSmoothingColor
    );
    game.wallShadowEffectCaches.clear();
    game.wallShadowEffectCaches.insert(
        game.wallShadowEffectCaches.begin(),
        game.curAreaData->edges.size(),
        EdgeOffsetCache()
    );
    updateOffsetEffectCaches(
        game.wallShadowEffectCaches,
        unordered_set<Vertex*>(
            game.curAreaData->vertexes.begin(),
            game.curAreaData->vertexes.end()
        ),
        doesEdgeHaveWallShadow,
        getWallShadowLength,
        getWallShadowColor
    );
    
    //TODO Uncomment this when replays are implemented.
    /*
    replayTimer = timer(
        GAMEPLAY::REPLAY_SAVE_FREQUENCY,
    [this] () {
        this->replayTimer.start();
        vector<mob*> obstacles; //TODO
        gameplayReplay.addState(
            leaders, pikminList, enemies, treasures, onions, obstacles,
            curLeaderIdx
        );
    }
    );
    replayTimer.start();
    gameplayReplay.clear();*/
    
    //Report any errors with the loading process.
    game.errors.reportAreaLoadErrors();
    
    if(game.perfMon) {
        game.perfMon->setAreaName(game.curAreaData->name);
        game.perfMon->leaveState();
    }
    
    enter();
    
    loading = false;
}


/**
 * @brief Loads all of the game's content.
 */
void GameplayState::loadGameContent() {
    game.content.reloadPacks();
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_GUI,
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Area manifests.
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    //Mob types.
    game.content.loadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_MOB_TYPE,
    },
    CONTENT_LOAD_LEVEL_FULL
    );
    
    //Register leader sub-group types.
    for(size_t p = 0; p < game.config.pikmin.order.size(); p++) {
        subgroupTypes.registerType(
            SUBGROUP_TYPE_CATEGORY_PIKMIN, game.config.pikmin.order[p],
            game.config.pikmin.order[p]->bmpIcon
        );
    }
    
    vector<string> toolTypesVector;
    for(auto &t : game.content.mobTypes.list.tool) {
        toolTypesVector.push_back(t.first);
    }
    sort(toolTypesVector.begin(), toolTypesVector.end());
    for(size_t t = 0; t < toolTypesVector.size(); t++) {
        ToolType* ttPtr = game.content.mobTypes.list.tool[toolTypesVector[t]];
        subgroupTypes.registerType(
            SUBGROUP_TYPE_CATEGORY_TOOL, ttPtr, ttPtr->bmpIcon
        );
    }
    
    subgroupTypes.registerType(SUBGROUP_TYPE_CATEGORY_LEADER);
}


/**
 * @brief Starts the fade out to leave the gameplay state.
 *
 * @param target Where to leave to.
 */
void GameplayState::startLeaving(const GAMEPLAY_LEAVE_TARGET target) {
    game.fadeMgr.startFade( false, [this, target] () { leave(target); });
}


/**
 * @brief Unloads the "gameplay" state from memory.
 */
void GameplayState::unload() {
    unloading = true;
    
    for(Player &player : players) {
        if(player.hud) {
            player.hud->gui.destroy();
            delete player.hud;
            player.hud = nullptr;
        }
        
        player.leaderIdx = INVALID;
        player.leaderPtr = nullptr;
        
        player.closeToInteractableToUse = nullptr;
        player.closeToNestToOpen = nullptr;
        player.closeToPikminToPluck = nullptr;
        player.closeToShipToHeal = nullptr;
        
        player.view.cam.setPos(Point());
        player.view.cam.setZoom(1.0f);
        
        player.leaderMovement.reset(); //TODO replace with a better solution.
    }
    
    while(!mobs.all.empty()) {
        deleteMob(*mobs.all.begin(), true);
    }
    
    if(lightmapBmp) {
        al_destroy_bitmap(lightmapBmp);
        lightmapBmp = nullptr;
    }
    
    missionRemainingMobIds.clear();
    pathMgr.clear();
    particles.clear();
    
    for(size_t t = 0; t < MAX_PLAYER_TEAMS; t++) {
        playerTeams[t].sprayStats.clear();
    }
    
    game.sysContent.anmSparks.clear();
    unloadGameContent();
    game.content.unloadCurrentArea(CONTENT_LOAD_LEVEL_FULL);
    
    if(bmpFog) {
        al_destroy_bitmap(bmpFog);
        bmpFog = nullptr;
    }
    
    if(msgBox) {
        delete msgBox;
        msgBox = nullptr;
    }
    if(onionMenu) {
        delete onionMenu;
        onionMenu = nullptr;
    }
    if(pauseMenu) {
        delete pauseMenu;
        pauseMenu = nullptr;
    }
    game.makerTools.infoPrintText.clear();
    
    unloading = false;
}


/**
 * @brief Unloads loaded game content.
 */
void GameplayState::unloadGameContent() {
    subgroupTypes.clear();
    
    game.content.unloadAll(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_AREA,
        CONTENT_TYPE_WEATHER_CONDITION,
        CONTENT_TYPE_MOB_TYPE,
        CONTENT_TYPE_MOB_ANIMATION,
        CONTENT_TYPE_SPIKE_DAMAGE_TYPE,
        CONTENT_TYPE_HAZARD,
        CONTENT_TYPE_SPRAY_TYPE,
        CONTENT_TYPE_STATUS_TYPE,
        CONTENT_TYPE_LIQUID,
        CONTENT_TYPE_GLOBAL_ANIMATION,
        CONTENT_TYPE_PARTICLE_GEN,
        CONTENT_TYPE_GUI,
    }
    );
}


/**
 * @brief Updates the list of leaders available to be controlled.
 */
void GameplayState::updateAvailableLeaders() {
    //Build the list.
    availableLeaders.clear();
    for(size_t l = 0; l < mobs.leaders.size(); l++) {
        if(mobs.leaders[l]->health <= 0.0f) continue;
        if(mobs.leaders[l]->toDelete) continue;
        if(mobs.leaders[l]->isStoredInsideMob()) continue;
        availableLeaders.push_back(mobs.leaders[l]);
    }
    
    if(availableLeaders.empty()) {
        return;
    }
    
    //Sort it so that it follows the expected leader order.
    //If there are multiple leaders of the same type, leaders with a lower
    //mob index number come first.
    std::sort(
        availableLeaders.begin(), availableLeaders.end(),
    [] (const Leader * l1, const Leader * l2) -> bool {
        size_t l1OrderIdx = INVALID;
        size_t l2OrderIdx = INVALID;
        for(size_t t = 0; t < game.config.leaders.order.size(); t++) {
            if(game.config.leaders.order[t] == l1->type) l1OrderIdx = t;
            if(game.config.leaders.order[t] == l2->type) l2OrderIdx = t;
        }
        if(l1OrderIdx == l2OrderIdx) {
            return l1->id < l2->id;
        }
        return l1OrderIdx < l2OrderIdx;
    }
    );
    
    //Update the current leader's index, which could've changed.
    for(Player &player : players) {
        for(size_t l = 0; l < availableLeaders.size(); l++) {
            if(availableLeaders[l] == player.leaderPtr) {
                player.leaderIdx = l;
                break;
            }
        }
    }
}


/**
 * @brief Updates the variables that indicate what the closest
 * group member of the standby subgroup is, for the current
 * standby subgroup, the previous, and the next.
 *
 * In the case all candidate members are out of reach,
 * this gets set to the closest. Otherwise, it gets set to the closest
 * and more mature one.
 * Sets to nullptr if there is no member of that subgroup available.
 *
 * @param player The player responsible.
 */
void GameplayState::updateClosestGroupMembers(Player* player) {
    player->closestGroupMember[BUBBLE_RELATION_PREVIOUS] = nullptr;
    player->closestGroupMember[BUBBLE_RELATION_CURRENT] = nullptr;
    player->closestGroupMember[BUBBLE_RELATION_NEXT] = nullptr;
    player->closestGroupMemberDistant = false;
    
    if(!player->leaderPtr) return;
    if(player->leaderPtr->group->members.empty()) {
        player->leaderPtr->updateThrowVariables();
        return;
    }
    
    //Get the closest group members for the three relevant subgroup types.
    SubgroupType* prevType;
    player->leaderPtr->group->getNextStandbyType(true, &prevType);
    
    if(prevType) {
        player->closestGroupMember[BUBBLE_RELATION_PREVIOUS] =
            getClosestGroupMember(player, prevType);
    }
    
    if(player->leaderPtr->group->curStandbyType) {
        player->closestGroupMember[BUBBLE_RELATION_CURRENT] =
            getClosestGroupMember(
                player,
                player->leaderPtr->group->curStandbyType,
                &player->closestGroupMemberDistant
            );
    }
    
    SubgroupType* nextType;
    player->leaderPtr->group->getNextStandbyType(false, &nextType);
    
    if(nextType) {
        player->closestGroupMember[BUBBLE_RELATION_NEXT] =
            getClosestGroupMember(player, nextType);
    }
    
    if(player->closestGroupMember[BUBBLE_RELATION_CURRENT]) {
        player->leaderPtr->updateThrowVariables();
    }
}


/**
 * @brief Constructs a new message box info object.
 *
 * @param text Text to display.
 * @param speakerIcon If not nullptr, use this bitmap to represent who
 * is talking.
 */
GameplayMessageBox::GameplayMessageBox(const string &text, ALLEGRO_BITMAP* speakerIcon):
    speakerIcon(speakerIcon) {
    
    string message = unescapeString(text);
    if(message.size() && message.back() == '\n') {
        message.pop_back();
    }
    vector<StringToken> tokens = tokenizeString(message);
    setStringTokenWidths(
        tokens, game.sysContent.fntStandard, game.sysContent.fntSlim,
        al_get_font_line_height(game.sysContent.fntStandard), true
    );
    
    vector<StringToken> line;
    for(size_t t = 0; t < tokens.size(); t++) {
        if(tokens[t].type == STRING_TOKEN_LINE_BREAK) {
            tokensPerLine.push_back(line);
            line.clear();
        } else {
            line.push_back(tokens[t]);
        }
    }
    if(!line.empty()) {
        tokensPerLine.push_back(line);
    }
}


/**
 * @brief Handles the user having pressed the button to continue the message,
 * or to skip to showing everything in the current section.
 */
void GameplayMessageBox::advance() {
    if(
        transitionTimer > 0.0f ||
        misinputProtectionTimer > 0.0f ||
        swipeTimer > 0.0f
    ) return;
    
    size_t lastToken = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t lineIdx = curSection * 3 + l;
        if(lineIdx >= tokensPerLine.size()) break;
        lastToken += tokensPerLine[lineIdx].size();
    }
    
    if(curToken >= lastToken + 1) {
        if(curSection >= ceil(tokensPerLine.size() / 3.0f) - 1) {
            //End of the message. Start closing the message box.
            close();
        } else {
            //Start swiping to go to the next section.
            swipeTimer = GAMEPLAY_MSG_BOX::TOKEN_SWIPE_DURATION;
        }
    } else {
        //Skip the text typing and show everything in this section.
        skippedAtToken = curToken;
        curToken = lastToken + 1;
    }
}


/**
 * @brief Closes the message box, even if it is still writing something.
 */
void GameplayMessageBox::close() {
    if(!transitionIn && transitionTimer > 0.0f) return;
    transitionIn = false;
    transitionTimer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayMessageBox::tick(float deltaT) {
    size_t tokensInSection = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t lineIdx = curSection * 3 + l;
        if(lineIdx >= tokensPerLine.size()) break;
        tokensInSection += tokensPerLine[lineIdx].size();
    }
    
    //Animate the swipe animation.
    if(swipeTimer > 0.0f) {
        swipeTimer -= deltaT;
        if(swipeTimer <= 0.0f) {
            //Go to the next section.
            swipeTimer = 0.0f;
            curSection++;
            totalTokenAnimTime = 0.0f;
            totalSkipAnimTime = 0.0f;
            skippedAtToken = INVALID;
        }
    }
    
    if(!transitionIn || transitionTimer == 0.0f) {
    
        //Animate the text.
        if(game.config.aestheticGen.gameplayMsgChInterval == 0.0f) {
            skippedAtToken = 0;
            curToken = tokensInSection + 1;
        } else {
            totalTokenAnimTime += deltaT;
            if(skippedAtToken == INVALID) {
                size_t prevToken = curToken;
                curToken =
                    totalTokenAnimTime /
                    game.config.aestheticGen.gameplayMsgChInterval;
                curToken =
                    std::min(curToken, tokensInSection + 1);
                if(
                    curToken == tokensInSection + 1 &&
                    prevToken != curToken
                ) {
                    //We've reached the last token organically.
                    //Start a misinput protection timer, so the player
                    //doesn't accidentally go to the next section when they
                    //were just trying to skip the text.
                    misinputProtectionTimer =
                        GAMEPLAY_MSG_BOX::MISINPUT_PROTECTION_DURATION;
                }
            } else {
                totalSkipAnimTime += deltaT;
            }
        }
        
    }
    
    //Animate the transition.
    transitionTimer -= deltaT;
    transitionTimer = std::max(0.0f, transitionTimer);
    if(!transitionIn && transitionTimer == 0.0f) {
        toDelete = true;
    }
    
    //Misinput protection logic.
    misinputProtectionTimer -= deltaT;
    misinputProtectionTimer = std::max(0.0f, misinputProtectionTimer);
    
    //Button opacity logic.
    if(
        transitionTimer == 0.0f &&
        misinputProtectionTimer == 0.0f &&
        swipeTimer == 0.0f &&
        curToken >= tokensInSection + 1
    ) {
        advanceButtonAlpha =
            std::min(
                advanceButtonAlpha +
                GAMEPLAY_MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * deltaT,
                1.0f
            );
    } else {
        advanceButtonAlpha =
            std::max(
                0.0f,
                advanceButtonAlpha -
                GAMEPLAY_MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * deltaT
            );
    }
}
