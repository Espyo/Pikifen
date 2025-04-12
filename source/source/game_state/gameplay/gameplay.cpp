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
 * @param type_idx Index number of the spray type.
 * @param amount Amount to change by.
 */
void GameplayState::changeSprayCount(
    size_t type_idx, signed int amount
) {
    sprayStats[type_idx].nrSprays =
        std::max(
            (signed int) sprayStats[type_idx].nrSprays + amount,
            (signed int) 0
        );
        
    GuiItem* spray_hud_item = nullptr;
    if(game.content.sprayTypes.list.size() > 2) {
        if(selectedSpray == type_idx) {
            spray_hud_item = hud->spray1Amount;
        }
    } else {
        if(type_idx == 0) {
            spray_hud_item = hud->spray1Amount;
        } else {
            spray_hud_item = hud->spray2Amount;
        }
    }
    if(spray_hud_item) {
        spray_hud_item->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
        );
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
    
    float regular_delta_t = game.deltaT;
    
    if(game.makerTools.changeSpeed) {
        game.deltaT *=
            game.makerTools.changeSpeedSettings[
                game.makerTools.changeSpeedSettingIdx
            ];
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
        game.statistics.gameplayTime += regular_delta_t;
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
    leaderMovement.reset(); //TODO replace with a better solution.
    
    //Zoom in on the reason, if possible.
    Point new_cam_pos = game.cam.targetPos;
    float new_cam_zoom = game.cam.targetZoom;
    if(cleared) {
        MissionGoal* goal =
            game.missionGoals[game.curAreaData->mission.goal];
        if(goal->getEndZoomData(this, &new_cam_pos, &new_cam_zoom)) {
            game.cam.targetPos = new_cam_pos;
            game.cam.targetZoom = new_cam_zoom;
        }
        
    } else {
        MissionFail* cond =
            game.missionFailConds[missionFailReason];
        if(cond->getEndZoomData(this, &new_cam_pos, &new_cam_zoom)) {
            game.cam.targetPos = new_cam_pos;
            game.cam.targetZoom = new_cam_zoom;
        }
    }
    
    if(cleared) {
        curBigMsg = BIG_MESSAGE_MISSION_CLEAR;
    } else {
        curBigMsg = BIG_MESSAGE_MISSION_FAILED;
    }
    bigMsgTime = 0.0f;
    hud->gui.startAnimation(
        GUI_MANAGER_ANIM_IN_TO_OUT,
        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/**
 * @brief Code to run when the state is entered, be it from the area menu, be it
 * from the result menu's "keep playing" option.
 */
void GameplayState::enter() {
    updateTransformations();
    
    lastEnemyKilledPos = Point(LARGE_FLOAT);
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
    
    hud->gui.hideItems();
    if(wentToResults) {
        game.fadeMgr.startFade(true, nullptr);
        if(pauseMenu) {
            pauseMenu->toDelete = true;
        }
    }
    
    readyForInput = false;
    
    game.mouseCursor.reset();
    leaderCursorW = game.mouseCursor.wPos;
    leaderCursorS = game.mouseCursor.sPos;
    
    notification.reset();
    
    if(curLeaderPtr) {
        curLeaderPtr->stopWhistling();
    }
    updateClosestGroupMembers();
}


/**
 * @brief Generates the bitmap that'll draw the fog fade effect.
 *
 * @param near_radius Until this radius, the fog is not present.
 * @param far_radius From this radius on, the fog is fully dense.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* GameplayState::generateFogBitmap(
    float near_radius, float far_radius
) {
    if(far_radius == 0) return nullptr;
    
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
    float near_ratio = near_radius / far_radius;
    
#define fill_pixel(x, row) \
    row[(x) * 4 + 0] = 255; \
    row[(x) * 4 + 1] = 255; \
    row[(x) * 4 + 2] = 255; \
    row[(x) * 4 + 3] = cur_a; \
    
    for(int y = 0; y < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); y++) {
        for(int x = 0; x < ceil(GAMEPLAY::FOG_BITMAP_SIZE / 2.0); x++) {
            //First, get how far this pixel is from the center.
            //Center = 0, radius or beyond = 1.
            float cur_ratio =
                Distance(
                    Point(x, y),
                    Point(
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0,
                        GAMEPLAY::FOG_BITMAP_SIZE / 2.0
                    )
                ).toFloat() / (GAMEPLAY::FOG_BITMAP_SIZE / 2.0);
            cur_ratio = std::min(cur_ratio, 1.0f);
            //Then, map that ratio to a different ratio that considers
            //the start of the "near" section as 0.
            cur_ratio =
                interpolateNumber(cur_ratio, near_ratio, 1.0f, 0.0f, 1.0f);
            //Finally, clamp the value and get the alpha.
            cur_ratio = std::clamp(cur_ratio, 0.0f, 1.0f);
            unsigned char cur_a = 255 * cur_ratio;
            
            //Save the memory location of the opposite row's pixels.
            unsigned char* opposite_row =
                row + region->pitch * (GAMEPLAY::FOG_BITMAP_SIZE - y - y - 1);
            fill_pixel(x, row);
            fill_pixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, row);
            fill_pixel(x, opposite_row);
            fill_pixel(GAMEPLAY::FOG_BITMAP_SIZE - x - 1, opposite_row);
        }
        row += region->pitch;
    }
    
#undef fill_pixel
    
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
        Pikmin* p_ptr = mobs.pikmin[p];
        if(filter && p_ptr->pikType != filter) continue;
        total++;
    }
    
    //Check Pikmin inside converters.
    for(size_t c = 0; c < mobs.converters.size(); c++) {
        Converter* c_ptr = mobs.converters[c];
        if(filter && c_ptr->currentType != filter) continue;
        total += c_ptr->amountInBuffer;
    }
    
    return total;
}


/**
 * @brief Returns how many Pikmin are in the group.
 *
 * @param filter If not nullptr, only return Pikmin matching this type.
 * @return The amount.
 */
size_t GameplayState::getAmountOfGroupPikmin(const PikminType* filter) {
    size_t total = 0;
    
    if(!curLeaderPtr) return 0;
    
    for(size_t m = 0; m < curLeaderPtr->group->members.size(); m++) {
        Mob* m_ptr = curLeaderPtr->group->members[m];
        if(m_ptr->type->category->id != MOB_CATEGORY_PIKMIN) continue;
        if(filter && m_ptr->type != filter) continue;
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
        Pikmin* p_ptr = mobs.pikmin[p];
        if(filter && p_ptr->type != filter) continue;
        if(
            p_ptr->fsm.curState->id == PIKMIN_STATE_IDLING ||
            p_ptr->fsm.curState->id == PIKMIN_STATE_IDLING_H
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
        Onion* o_ptr = mobs.onions[o];
        for(
            size_t t = 0;
            t < o_ptr->oniType->nest->pik_types.size();
            t++
        ) {
            if(filter && o_ptr->oniType->nest->pik_types[t] != filter) {
                continue;
            }
            for(size_t m = 0; m < N_MATURITIES; m++) {
                total += (long) o_ptr->nest->pikmin_inside[t][m];
            }
        }
    }
    
    //Check ships.
    for(size_t s = 0; s < mobs.ships.size(); s++) {
        Ship* s_ptr = mobs.ships[s];
        if(!s_ptr->nest) continue;
        for(
            size_t t = 0;
            t < s_ptr->shiType->nest->pik_types.size();
            t++
        ) {
            if(filter && s_ptr->shiType->nest->pik_types[t] != filter) {
                continue;
            }
            for(size_t m = 0; m < N_MATURITIES; m++) {
                total += (long) s_ptr->nest->pikmin_inside[t][m];
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
 * @param type Type to search for.
 * @param distant If not nullptr, whether all members are unreachable is
 * returned here.
 * @return The closest member, or nullptr if there is no member
 * of that subgroup available to grab.
 */
Mob* GameplayState::getClosestGroupMember(
    const SubgroupType* type, bool* distant
) {
    if(!curLeaderPtr) return nullptr;
    
    Mob* result = nullptr;
    
    //Closest members so far for each maturity.
    Distance closest_dists[N_MATURITIES];
    Mob* closest_ptrs[N_MATURITIES];
    bool can_grab_closest[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; m++) {
        closest_ptrs[m] = nullptr;
        can_grab_closest[m] = false;
    }
    
    //Fetch the closest, for each maturity.
    size_t n_members = curLeaderPtr->group->members.size();
    for(size_t m = 0; m < n_members; m++) {
    
        Mob* member_ptr = curLeaderPtr->group->members[m];
        if(member_ptr->subgroupTypePtr != type) {
            continue;
        }
        
        unsigned char maturity = 0;
        if(member_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
            maturity = ((Pikmin*) member_ptr)->maturity;
        }
        bool can_grab = curLeaderPtr->canGrabGroupMember(member_ptr);
        
        if(!can_grab && can_grab_closest[maturity]) {
            //Skip if we'd replace a grabbable Pikmin with a non-grabbable one.
            continue;
        }
        
        Distance d(curLeaderPtr->pos, member_ptr->pos);
        
        if(
            (can_grab && !can_grab_closest[maturity]) ||
            !closest_ptrs[maturity] ||
            d < closest_dists[maturity]
        ) {
            closest_dists[maturity] = d;
            closest_ptrs[maturity] = member_ptr;
            can_grab_closest[maturity] = can_grab;
            
        }
    }
    
    //Now, try to get the one with the highest maturity within reach.
    Distance closest_dist;
    for(unsigned char m = 0; m < N_MATURITIES; m++) {
        if(!closest_ptrs[2 - m]) continue;
        if(!can_grab_closest[2 - m]) continue;
        result = closest_ptrs[2 - m];
        closest_dist = closest_dists[2 - m];
        break;
    }
    
    if(distant) *distant = !result;
    
    if(!result) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; m++) {
            if(!closest_ptrs[m]) continue;
            
            if(!result || closest_dists[m] < closest_dist) {
                result = closest_ptrs[m];
                closest_dist = closest_dists[m];
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
    hud->gui.handleAllegroEvent(ev);
    
}


/**
 * @brief Initializes the HUD.
 */
void GameplayState::initHud() {
    hud = new Hud();
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
    for(size_t s = 0; s < game.content.sprayTypes.list.size(); s++) {
        sprayStats.push_back(SprayStats());
    }
    
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
    enemyDeaths = 0;
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
    
    auto spark_anim_db_it =
        game.content.globalAnimDbs.list.find(
            game.sysContentNames.anmSparks
        );
    if(spark_anim_db_it == game.content.globalAnimDbs.list.end()) {
        game.errors.report(
            "Unknown global animation \"" + game.sysContentNames.anmSparks +
            "\" when trying to load the leader damage sparks!"
        );
    } else {
        game.sysContent.anmSparks.initToFirstAnim(
            &spark_anim_db_it->second
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
    
    vector<Mob*> mobs_per_gen;
    
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* m_ptr = game.curAreaData->mobGenerators[m];
        bool valid = true;
        
        if(!m_ptr->type) {
            valid = false;
        } else if(
            m_ptr->type->category->id == MOB_CATEGORY_PIKMIN &&
            game.states.gameplay->mobs.pikmin.size() >=
            game.config.rules.maxPikminInField
        ) {
            valid = false;
        }
        
        if(valid) {
            Mob* new_mob =
                createMob(
                    m_ptr->type->category, m_ptr->pos, m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                );
            mobs_per_gen.push_back(new_mob);
        } else {
            mobs_per_gen.push_back(nullptr);
        }
    }
    
    //Mob links.
    //Because mobs can create other mobs when loaded, mob gen index X
    //does not necessarily correspond to mob index X. Hence, we need
    //to keep the pointers to the created mobs in a vector, and use this
    //to link the mobs by (generator) index.
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* gen_ptr = game.curAreaData->mobGenerators[m];
        Mob* mob_ptr = mobs_per_gen[m];
        if(!mob_ptr) continue;
        
        for(size_t l = 0; l < gen_ptr->linkIdxs.size(); l++) {
            size_t link_target_gen_idx = gen_ptr->linkIdxs[l];
            Mob* link_target_mob_ptr = mobs_per_gen[link_target_gen_idx];
            mob_ptr->links.push_back(link_target_mob_ptr);
        }
    }
    
    //Mobs stored inside other. Same logic as mob links.
    for(size_t m = 0; m < game.curAreaData->mobGenerators.size(); m++) {
        MobGen* holdee_gen_ptr = game.curAreaData->mobGenerators[m];
        if(holdee_gen_ptr->storedInside == INVALID) continue;
        Mob* holdee_ptr = mobs_per_gen[m];
        Mob* holder_mob_ptr = mobs_per_gen[holdee_gen_ptr->storedInside];
        holder_mob_ptr->storeMobInside(holdee_ptr);
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
        size_t priority_l1 =
        find(
            game.config.leaders.order.begin(),
            game.config.leaders.order.end(), l1->leaType
        ) -
        game.config.leaders.order.begin();
        size_t priority_l2 =
        find(
            game.config.leaders.order.begin(),
            game.config.leaders.order.end(), l2->leaType
        ) -
        game.config.leaders.order.begin();
        return priority_l1 < priority_l2;
    }
    );
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
    //In case a leader is stored in another mob,
    //update the available list.
    updateAvailableLeaders();
    
    curLeaderIdx = INVALID;
    curLeaderPtr = nullptr;
    startingNrOfLeaders = mobs.leaders.size();
    
    if(!mobs.leaders.empty()) {
        changeToNextLeader(true, false, false);
    }
    
    if(curLeaderPtr) {
        game.cam.setPos(curLeaderPtr->pos);
    } else {
        game.cam.setPos(Point());
    }
    game.cam.set_zoom(game.options.advanced.zoomMidLevel);
    
    //Memorize mobs required by the mission.
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        unordered_set<size_t> mission_required_mob_gen_idxs;
        
        if(game.curAreaData->mission.goalAllMobs) {
            for(size_t m = 0; m < mobs_per_gen.size(); m++) {
                if(
                    mobs_per_gen[m] &&
                    game.missionGoals[game.curAreaData->mission.goal]->
                    isMobApplicable(mobs_per_gen[m]->type)
                ) {
                    mission_required_mob_gen_idxs.insert(m);
                }
            }
            
        } else {
            mission_required_mob_gen_idxs =
                game.curAreaData->mission.goalMobIdxs;
        }
        
        for(size_t i : mission_required_mob_gen_idxs) {
            missionRemainingMobIds.insert(mobs_per_gen[i]->id);
        }
        missionRequiredMobAmount = missionRemainingMobIds.size();
        
        if(game.curAreaData->mission.goal == MISSION_GOAL_COLLECT_TREASURE) {
            //Since the collect treasure goal can accept piles and resources
            //meant to add treasure points, we'll need some special treatment.
            for(size_t i : mission_required_mob_gen_idxs) {
                if(
                    mobs_per_gen[i]->type->category->id ==
                    MOB_CATEGORY_PILES
                ) {
                    Pile* pil_ptr = (Pile*) mobs_per_gen[i];
                    goalTreasuresTotal += pil_ptr->amount;
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
        Pile* p_ptr = mobs.piles[p];
        ResourceType* res_type = p_ptr->pilType->contents;
        if(
            res_type->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasuresTotal += p_ptr->amount;
        treasurePointsTotal +=
            p_ptr->amount * res_type->pointAmount;
    }
    for(size_t r = 0; r < mobs.resources.size(); r++) {
        Resource* r_ptr = mobs.resources[r];
        if(
            r_ptr->resType->deliveryResult !=
            RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
        ) {
            continue;
        }
        treasuresTotal++;
        treasurePointsTotal += r_ptr->resType->pointAmount;
    }
    
    //Figure out the total amount of enemies and their points.
    enemyTotal = mobs.enemies.size();
    for(size_t e = 0; e < mobs.enemies.size(); e++) {
        enemyPointsTotal += mobs.enemies[e]->eneType->points;
    }
    
    //Initialize the area's active cells.
    float area_width =
        game.curAreaData->bmap.nCols * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    float area_height =
        game.curAreaData->bmap.nRows * GEOMETRY::BLOCKMAP_BLOCK_SIZE;
    size_t nr_area_cell_cols =
        ceil(area_width / GEOMETRY::AREA_CELL_SIZE) + 1;
    size_t nr_area_cell_rows =
        ceil(area_height / GEOMETRY::AREA_CELL_SIZE) + 1;
        
    areaActiveCells.clear();
    areaActiveCells.assign(
        nr_area_cell_cols, vector<bool>(nr_area_cell_rows, false)
    );
    
    //Initialize some other things.
    pathMgr.handleAreaLoad();
    
    initHud();
    
    dayMinutes = game.curAreaData->dayTimeStart;
    
    map<string, string> spray_strs =
        getVarMap(game.curAreaData->sprayAmounts);
        
    for(auto &s : spray_strs) {
        size_t spray_idx = 0;
        for(; spray_idx < game.config.misc.sprayOrder.size(); spray_idx++) {
            if(game.config.misc.sprayOrder[spray_idx]->manifest->internalName == s.first) {
                break;
            }
        }
        if(spray_idx == game.content.sprayTypes.list.size()) {
            game.errors.report(
                "Unknown spray type \"" + s.first + "\", "
                "while trying to set the starting number of sprays for "
                "area \"" + game.curAreaData->name + "\"!", nullptr
            );
            continue;
        }
        
        sprayStats[spray_idx].nrSprays = s2i(s.second);
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
            leaders, pikmin_list, enemies, treasures, onions, obstacles,
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
    
    vector<string> tool_types_vector;
    for(auto &t : game.content.mobTypes.list.tool) {
        tool_types_vector.push_back(t.first);
    }
    sort(tool_types_vector.begin(), tool_types_vector.end());
    for(size_t t = 0; t < tool_types_vector.size(); t++) {
        ToolType* tt_ptr = game.content.mobTypes.list.tool[tool_types_vector[t]];
        subgroupTypes.registerType(
            SUBGROUP_TYPE_CATEGORY_TOOL, tt_ptr, tt_ptr->bmpIcon
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
    
    if(hud) {
        hud->gui.destroy();
        delete hud;
        hud = nullptr;
    }
    
    curLeaderIdx = INVALID;
    curLeaderPtr = nullptr;
    
    closeToInteractableToUse = nullptr;
    closeToNestToOpen = nullptr;
    closeToPikminToPluck = nullptr;
    closeToShipToHeal = nullptr;
    
    game.cam.setPos(Point());
    game.cam.set_zoom(1.0f);
    
    while(!mobs.all.empty()) {
        deleteMob(*mobs.all.begin(), true);
    }
    
    if(lightmapBmp) {
        al_destroy_bitmap(lightmapBmp);
        lightmapBmp = nullptr;
    }
    
    missionRemainingMobIds.clear();
    pathMgr.clear();
    sprayStats.clear();
    particles.clear();
    
    leaderMovement.reset(); //TODO replace with a better solution.
    
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
        size_t l1_order_idx = INVALID;
        size_t l2_order_idx = INVALID;
        for(size_t t = 0; t < game.config.leaders.order.size(); t++) {
            if(game.config.leaders.order[t] == l1->type) l1_order_idx = t;
            if(game.config.leaders.order[t] == l2->type) l2_order_idx = t;
        }
        if(l1_order_idx == l2_order_idx) {
            return l1->id < l2->id;
        }
        return l1_order_idx < l2_order_idx;
    }
    );
    
    //Update the current leader's index, which could've changed.
    for(size_t l = 0; l < availableLeaders.size(); l++) {
        if(availableLeaders[l] == curLeaderPtr) {
            curLeaderIdx = l;
            break;
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
 */
void GameplayState::updateClosestGroupMembers() {
    closestGroupMember[BUBBLE_RELATION_PREVIOUS] = nullptr;
    closestGroupMember[BUBBLE_RELATION_CURRENT] = nullptr;
    closestGroupMember[BUBBLE_RELATION_NEXT] = nullptr;
    closestGroupMemberDistant = false;
    
    if(!curLeaderPtr) return;
    if(curLeaderPtr->group->members.empty()) {
        curLeaderPtr->updateThrowVariables();
        return;
    }
    
    //Get the closest group members for the three relevant subgroup types.
    SubgroupType* prev_type;
    curLeaderPtr->group->getNextStandbyType(true, &prev_type);
    
    if(prev_type) {
        closestGroupMember[BUBBLE_RELATION_PREVIOUS] =
            getClosestGroupMember(prev_type);
    }
    
    if(curLeaderPtr->group->curStandbyType) {
        closestGroupMember[BUBBLE_RELATION_CURRENT] =
            getClosestGroupMember(
                curLeaderPtr->group->curStandbyType,
                &closestGroupMemberDistant
            );
    }
    
    SubgroupType* next_type;
    curLeaderPtr->group->getNextStandbyType(false, &next_type);
    
    if(next_type) {
        closestGroupMember[BUBBLE_RELATION_NEXT] =
            getClosestGroupMember(next_type);
    }
    
    if(closestGroupMember[BUBBLE_RELATION_CURRENT]) {
        curLeaderPtr->updateThrowVariables();
    }
}


/**
 * @brief Updates the transformations, with the current camera coordinates,
 * zoom, etc.
 */
void GameplayState::updateTransformations() {
    //World coordinates to screen coordinates.
    game.worldToScreenTransform = game.identityTransform;
    al_translate_transform(
        &game.worldToScreenTransform,
        -game.cam.pos.x + game.winW / 2.0 / game.cam.zoom,
        -game.cam.pos.y + game.winH / 2.0 / game.cam.zoom
    );
    al_scale_transform(
        &game.worldToScreenTransform, game.cam.zoom, game.cam.zoom
    );
    
    //Screen coordinates to world coordinates.
    game.screenToWorldTransform = game.worldToScreenTransform;
    al_invert_transform(&game.screenToWorldTransform);
}


/**
 * @brief Constructs a new message box info object.
 *
 * @param text Text to display.
 * @param speaker_icon If not nullptr, use this bitmap to represent who
 * is talking.
 */
GameplayMessageBox::GameplayMessageBox(const string &text, ALLEGRO_BITMAP* speaker_icon):
    speakerIcon(speaker_icon) {
    
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
    
    size_t last_token = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t line_idx = curSection * 3 + l;
        if(line_idx >= tokensPerLine.size()) break;
        last_token += tokensPerLine[line_idx].size();
    }
    
    if(curToken >= last_token + 1) {
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
        curToken = last_token + 1;
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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void GameplayMessageBox::tick(float delta_t) {
    size_t tokens_in_section = 0;
    for(size_t l = 0; l < 3; l++) {
        size_t line_idx = curSection * 3 + l;
        if(line_idx >= tokensPerLine.size()) break;
        tokens_in_section += tokensPerLine[line_idx].size();
    }
    
    //Animate the swipe animation.
    if(swipeTimer > 0.0f) {
        swipeTimer -= delta_t;
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
            curToken = tokens_in_section + 1;
        } else {
            totalTokenAnimTime += delta_t;
            if(skippedAtToken == INVALID) {
                size_t prev_token = curToken;
                curToken =
                    totalTokenAnimTime /
                    game.config.aestheticGen.gameplayMsgChInterval;
                curToken =
                    std::min(curToken, tokens_in_section + 1);
                if(
                    curToken == tokens_in_section + 1 &&
                    prev_token != curToken
                ) {
                    //We've reached the last token organically.
                    //Start a misinput protection timer, so the player
                    //doesn't accidentally go to the next section when they
                    //were just trying to skip the text.
                    misinputProtectionTimer =
                        GAMEPLAY_MSG_BOX::MISINPUT_PROTECTION_DURATION;
                }
            } else {
                totalSkipAnimTime += delta_t;
            }
        }
        
    }
    
    //Animate the transition.
    transitionTimer -= delta_t;
    transitionTimer = std::max(0.0f, transitionTimer);
    if(!transitionIn && transitionTimer == 0.0f) {
        toDelete = true;
    }
    
    //Misinput protection logic.
    misinputProtectionTimer -= delta_t;
    misinputProtectionTimer = std::max(0.0f, misinputProtectionTimer);
    
    //Button opacity logic.
    if(
        transitionTimer == 0.0f &&
        misinputProtectionTimer == 0.0f &&
        swipeTimer == 0.0f &&
        curToken >= tokens_in_section + 1
    ) {
        advanceButtonAlpha =
            std::min(
                advanceButtonAlpha +
                GAMEPLAY_MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * delta_t,
                1.0f
            );
    } else {
        advanceButtonAlpha =
            std::max(
                0.0f,
                advanceButtonAlpha -
                GAMEPLAY_MSG_BOX::ADVANCE_BUTTON_FADE_SPEED * delta_t
            );
    }
}
