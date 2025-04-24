/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main gameplay logic.
 */

#include <algorithm>
#include <unordered_set>

#include "gameplay.h"

#include "../../content/mob/group_task.h"
#include "../../content/mob/pikmin.h"
#include "../../content/mob/tool.h"
#include "../../core/const.h"
#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Ticks the logic of aesthetic things regarding the leader.
 * If the game is paused, these can be frozen in place without
 * any negative impact.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayState::doAestheticLeaderLogic(float deltaT) {
    if(!curLeaderPtr) return;
    
    //Swarming arrows.
    if(swarmMagnitude) {
        curLeaderPtr->swarmNextArrowTimer.tick(deltaT);
    }
    
    Distance leaderToCursorDist(curLeaderPtr->pos, leaderCursorW);
    for(size_t a = 0; a < curLeaderPtr->swarmArrows.size(); ) {
        curLeaderPtr->swarmArrows[a] +=
            GAMEPLAY::SWARM_ARROW_SPEED * deltaT;
            
        Distance maxDist =
            (swarmMagnitude > 0) ?
            Distance(game.config.rules.cursorMaxDist * swarmMagnitude) :
            leaderToCursorDist;
            
        if(maxDist < curLeaderPtr->swarmArrows[a]) {
            curLeaderPtr->swarmArrows.erase(
                curLeaderPtr->swarmArrows.begin() + a
            );
        } else {
            a++;
        }
    }
    
    //Whistle.
    float whistleDist;
    Point whistlePos;
    
    if(leaderToCursorDist > game.config.rules.whistleMaxDist) {
        whistleDist = game.config.rules.whistleMaxDist;
        float whistleAngle =
            getAngle(curLeaderPtr->pos, leaderCursorW);
        whistlePos = angleToCoordinates(whistleAngle, whistleDist);
        whistlePos += curLeaderPtr->pos;
    } else {
        whistleDist = leaderToCursorDist.toFloat();
        whistlePos = leaderCursorW;
    }
    
    whistle.tick(
        deltaT, whistlePos,
        curLeaderPtr->leaType->whistleRange, whistleDist
    );
    
    //Where the cursor is.
    cursorHeightDiffLight = 0;
    
    if(leaderToCursorDist > game.config.rules.throwMaxDist) {
        float throwAngle =
            getAngle(curLeaderPtr->pos, leaderCursorW);
        throwDest =
            angleToCoordinates(throwAngle, game.config.rules.throwMaxDist);
        throwDest += curLeaderPtr->pos;
    } else {
        throwDest = leaderCursorW;
    }
    
    throwDestMob = nullptr;
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* mPtr = mobs.all[m];
        if(!BBoxCheck(throwDest, mPtr->pos, mPtr->physicalSpan)) {
            //Too far away; of course the cursor isn't on it.
            continue;
        }
        if(!mPtr->type->pushable && !mPtr->type->walkable) {
            //If it doesn't push and can't be walked on, there's probably
            //nothing really for the Pikmin to land on top of.
            continue;
        }
        if(
            throwDestMob &&
            mPtr->z + mPtr->height <
            throwDestMob->z + throwDestMob->height
        ) {
            //If this mob is lower than the previous known "under cursor" mob,
            //then forget it.
            continue;
        }
        if(!mPtr->isPointOn(throwDest)) {
            //The cursor is not really on top of this mob.
            continue;
        }
        
        throwDestMob = mPtr;
    }
    
    leaderCursorSector =
        getSector(leaderCursorW, nullptr, true);
        
    throwDestSector =
        getSector(throwDest, nullptr, true);
        
    if(leaderCursorSector) {
        cursorHeightDiffLight =
            (leaderCursorSector->z - curLeaderPtr->z) * 0.001;
        cursorHeightDiffLight =
            std::clamp(cursorHeightDiffLight, -0.1f, 0.1f);
    }
    
}


/**
 * @brief Ticks the logic of aesthetic things.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayState::doAestheticLogic(float deltaT) {
    //Leader stuff.
    doAestheticLeaderLogic(deltaT);
    
    //Specific animations.
    game.sysContent.anmSparks.tick(deltaT);
}


/**
 * @brief Ticks the logic of leader gameplay-related things.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayState::doGameplayLeaderLogic(float deltaT) {
    if(!curLeaderPtr) return;
    
    if(game.perfMon) {
        game.perfMon->startMeasurement("Logic -- Current leader");
    }
    
    if(curLeaderPtr->toDelete) {
        game.states.gameplay->updateAvailableLeaders();
        changeToNextLeader(true, true, true);
    }
    
    /********************
    *              ***  *
    *   Whistle   * O * *
    *              ***  *
    ********************/
    
    if(
        whistle.whistling &&
        whistle.radius < curLeaderPtr->leaType->whistleRange
    ) {
        whistle.radius += game.config.rules.whistleGrowthSpeed * deltaT;
        if(whistle.radius > curLeaderPtr->leaType->whistleRange) {
            whistle.radius = curLeaderPtr->leaType->whistleRange;
        }
    }
    
    //Current leader movement.
    Point dummyCoords;
    float dummyAngle;
    float leaderMoveMagnitude;
    leaderMovement.getInfo(
        &dummyCoords, &dummyAngle, &leaderMoveMagnitude
    );
    if(leaderMoveMagnitude < 0.75) {
        curLeaderPtr->fsm.runEvent(
            LEADER_EV_MOVE_END, (void*) &leaderMovement
        );
    } else {
        curLeaderPtr->fsm.runEvent(
            LEADER_EV_MOVE_START, (void*) &leaderMovement
        );
    }
    
    if(curInterlude == INTERLUDE_NONE) {
        //Adjust the camera position.
        float leaderWeight = 1.0f;
        float cursorWeight = game.options.misc.cursorCamWeight;
        float groupWeight = 0.0f;
        
        Point groupCenter = curLeaderPtr->pos;
        if(!curLeaderPtr->group->members.empty()) {
            Point tl = curLeaderPtr->group->members[0]->pos;
            Point br = tl;
            for(size_t m = 1; m < curLeaderPtr->group->members.size(); m++) {
                Mob* member = curLeaderPtr->group->members[m];
                updateMinMaxCoords(tl, br, member->pos);
            }
            groupCenter.x = (tl.x + br.x) / 2.0f;
            groupCenter.y = (tl.y + br.y) / 2.0f;
            groupWeight = 0.1f;
        }
        
        float weightSums = leaderWeight + cursorWeight + groupWeight;
        if(weightSums == 0.0f) weightSums = 0.01f;
        leaderWeight /= weightSums;
        cursorWeight /= weightSums;
        groupWeight /= weightSums;
        
        game.view.cam.targetPos =
            curLeaderPtr->pos * leaderWeight +
            leaderCursorW * cursorWeight +
            groupCenter * groupWeight;
    }
    
    //Check what to show on the notification, if anything.
    notification.setEnabled(false);
    
    bool notificationDone = false;
    
    //Lying down stop notification.
    if(
        !notificationDone &&
        curLeaderPtr->carryInfo
    ) {
        notification.setEnabled(true);
        notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Get up",
            Point(
                curLeaderPtr->pos.x,
                curLeaderPtr->pos.y - curLeaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    //Get up notification.
    if(
        !notificationDone &&
        curLeaderPtr->fsm.curState->id == LEADER_STATE_KNOCKED_DOWN
    ) {
        notification.setEnabled(true);
        notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Get up",
            Point(
                curLeaderPtr->pos.x,
                curLeaderPtr->pos.y - curLeaderPtr->radius
            )
        );
        notificationDone = true;
    }
    //Auto-throw stop notification.
    if(
        !notificationDone &&
        curLeaderPtr->autoThrowRepeater.time != LARGE_FLOAT &&
        game.options.controls.autoThrowMode == AUTO_THROW_MODE_TOGGLE
    ) {
        notification.setEnabled(true);
        notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_THROW).inputSource,
            "Stop throwing",
            Point(
                curLeaderPtr->pos.x,
                curLeaderPtr->pos.y - curLeaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    //Pluck stop notification.
    if(
        !notificationDone &&
        curLeaderPtr->autoPlucking
    ) {
        notification.setEnabled(true);
        notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Stop",
            Point(
                curLeaderPtr->pos.x,
                curLeaderPtr->pos.y - curLeaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    //Go Here stop notification.
    if(
        !notificationDone &&
        curLeaderPtr->midGoHere
    ) {
        notification.setEnabled(true);
        notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Stop",
            Point(
                curLeaderPtr->pos.x,
                curLeaderPtr->pos.y - curLeaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    if(!curLeaderPtr->autoPlucking) {
        Distance closestD;
        Distance d;
        
        //Ship healing notification.
        closeToShipToHeal = nullptr;
        for(size_t s = 0; s < mobs.ships.size(); s++) {
            Ship* sPtr = mobs.ships[s];
            d = Distance(curLeaderPtr->pos, sPtr->pos);
            if(!sPtr->isLeaderOnCp(curLeaderPtr)) {
                continue;
            }
            if(curLeaderPtr->health == curLeaderPtr->maxHealth) {
                continue;
            }
            if(!sPtr->shiType->canHeal) {
                continue;
            }
            if(d < closestD || !closeToShipToHeal) {
                closeToShipToHeal = sPtr;
                closestD = d;
                notification.setEnabled(true);
                notification.setContents(
                    game.controls.findBind(PLAYER_ACTION_TYPE_THROW).inputSource,
                    "Repair suit",
                    Point(
                        closeToShipToHeal->pos.x,
                        closeToShipToHeal->pos.y -
                        closeToShipToHeal->radius
                    )
                );
                notificationDone = true;
            }
        }
        
        //Interactable mob notification.
        closestD = 0;
        d = 0;
        closeToInteractableToUse = nullptr;
        if(!notificationDone) {
            for(size_t i = 0; i < mobs.interactables.size(); i++) {
                d = Distance(curLeaderPtr->pos, mobs.interactables[i]->pos);
                if(d > mobs.interactables[i]->intType->triggerRange) {
                    continue;
                }
                if(d < closestD || !closeToInteractableToUse) {
                    closeToInteractableToUse = mobs.interactables[i];
                    closestD = d;
                    notification.setEnabled(true);
                    notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        closeToInteractableToUse->intType->promptText,
                        Point(
                            closeToInteractableToUse->pos.x,
                            closeToInteractableToUse->pos.y -
                            closeToInteractableToUse->radius
                        )
                    );
                    notificationDone = true;
                }
            }
        }
        
        //Pikmin pluck notification.
        closestD = 0;
        d = 0;
        closeToPikminToPluck = nullptr;
        if(!notificationDone) {
            Pikmin* p = getClosestSprout(curLeaderPtr->pos, &d, false);
            if(p && d <= game.config.leaders.pluckRange) {
                closeToPikminToPluck = p;
                notification.setEnabled(true);
                notification.setContents(
                    game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                    inputSource,
                    "Pluck",
                    Point(
                        p->pos.x,
                        p->pos.y -
                        p->radius
                    )
                );
                notificationDone = true;
            }
        }
        
        //Nest open notification.
        closestD = 0;
        d = 0;
        closeToNestToOpen = nullptr;
        if(!notificationDone) {
            for(size_t o = 0; o < mobs.onions.size(); o++) {
                d = Distance(curLeaderPtr->pos, mobs.onions[o]->pos);
                if(d > game.config.leaders.onionOpenRange) continue;
                if(d < closestD || !closeToNestToOpen) {
                    closeToNestToOpen = mobs.onions[o]->nest;
                    closestD = d;
                    notification.setEnabled(true);
                    notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        "Check",
                        Point(
                            closeToNestToOpen->mPtr->pos.x,
                            closeToNestToOpen->mPtr->pos.y -
                            closeToNestToOpen->mPtr->radius
                        )
                    );
                    notificationDone = true;
                }
            }
            for(size_t s = 0; s < mobs.ships.size(); s++) {
                d = Distance(curLeaderPtr->pos, mobs.ships[s]->pos);
                if(!mobs.ships[s]->isLeaderOnCp(curLeaderPtr)) {
                    continue;
                }
                if(mobs.ships[s]->shiType->nest->pikTypes.empty()) {
                    continue;
                }
                if(d < closestD || !closeToNestToOpen) {
                    closeToNestToOpen = mobs.ships[s]->nest;
                    closestD = d;
                    notification.setEnabled(true);
                    notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        "Check",
                        Point(
                            closeToNestToOpen->mPtr->pos.x,
                            closeToNestToOpen->mPtr->pos.y -
                            closeToNestToOpen->mPtr->radius
                        )
                    );
                    notificationDone = true;
                }
            }
        }
    }
    
    notification.tick(deltaT);
    
    /********************
    *             .-.   *
    *   Cursor   ( = )> *
    *             `-´   *
    ********************/
    
    Point mouseCursorSpeed;
    float dummyMagnitude;
    cursorMovement.getInfo(
        &mouseCursorSpeed, &dummyAngle, &dummyMagnitude
    );
    mouseCursorSpeed =
        mouseCursorSpeed * deltaT* game.options.controls.cursorSpeed;
        
    leaderCursorW = game.view.cursorWorldPos;
    
    float cursorAngle = getAngle(curLeaderPtr->pos, leaderCursorW);
    
    Distance leaderToCursorDist(curLeaderPtr->pos, leaderCursorW);
    if(leaderToCursorDist > game.config.rules.cursorMaxDist) {
        //Cursor goes beyond the range limit.
        leaderCursorW.x =
            curLeaderPtr->pos.x +
            (cos(cursorAngle) * game.config.rules.cursorMaxDist);
        leaderCursorW.y =
            curLeaderPtr->pos.y +
            (sin(cursorAngle) * game.config.rules.cursorMaxDist);
            
        if(mouseCursorSpeed.x != 0 || mouseCursorSpeed.y != 0) {
            //If we're speeding the mouse cursor (via analog stick),
            //don't let it go beyond the edges.
            game.view.cursorWorldPos = leaderCursorW;
            game.mouseCursor.winPos = game.view.cursorWorldPos;
            al_transform_coordinates(
                &game.view.worldToWindowTransform,
                &game.mouseCursor.winPos.x, &game.mouseCursor.winPos.y
            );
        }
    }
    
    leaderCursorWin = leaderCursorW;
    al_transform_coordinates(
        &game.view.worldToWindowTransform,
        &leaderCursorWin.x, &leaderCursorWin.y
    );
    
    
    /***********************************
    *                             ***  *
    *   Current leader's group   ****O *
    *                             ***  *
    ************************************/
    
    updateClosestGroupMembers();
    if(!curLeaderPtr->holding.empty()) {
        closestGroupMember[BUBBLE_RELATION_CURRENT] = curLeaderPtr->holding[0];
    }
    
    float oldSwarmMagnitude = swarmMagnitude;
    Point swarmCoords;
    float newSwarmAngle;
    swarmMovement.getInfo(
        &swarmCoords, &newSwarmAngle, &swarmMagnitude
    );
    if(swarmMagnitude > 0) {
        //This stops arrows that were fading away to the left from
        //turning to angle 0 because the magnitude reached 0.
        swarmAngle = newSwarmAngle;
    }
    
    if(swarmCursor) {
        swarmAngle = cursorAngle;
        leaderToCursorDist = Distance(curLeaderPtr->pos, leaderCursorW);
        swarmMagnitude =
            leaderToCursorDist.toFloat() / game.config.rules.cursorMaxDist;
    }
    
    if(oldSwarmMagnitude != swarmMagnitude) {
        if(swarmMagnitude != 0) {
            curLeaderPtr->signalSwarmStart();
        } else {
            curLeaderPtr->signalSwarmEnd();
        }
    }
    
    
    /*******************
    *                  *
    *   Others   o o o *
    *                  *
    ********************/
    
    //Closest enemy check for the music mix track.
    if(
        game.states.gameplay->mobs.enemies.size() > 0 &&
        curInterlude == INTERLUDE_NONE
    ) {
        bool nearEnemy = false;
        bool nearBoss = false;
        isNearEnemyAndBoss(&nearEnemy, &nearBoss);
        
        if(nearEnemy) {
            game.audio.markMixTrackStatus(MIX_TRACK_TYPE_ENEMY);
        }
        
        if(nearBoss) {
            switch(bossMusicState) {
            case BOSS_MUSIC_STATE_NEVER_PLAYED: {
                game.audio.setCurrentSong(game.sysContentNames.sngBoss, true, false);
                bossMusicState = BOSS_MUSIC_STATE_PLAYING;
                break;
            } case BOSS_MUSIC_STATE_PAUSED: {
            } case BOSS_MUSIC_STATE_VICTORY: {
                game.audio.setCurrentSong(game.sysContentNames.sngBoss, false);
                bossMusicState = BOSS_MUSIC_STATE_PLAYING;
            } default: {
                break;
            }
            }
        } else {
            switch(bossMusicState) {
            case BOSS_MUSIC_STATE_PLAYING: {
                game.audio.setCurrentSong(game.curAreaData->songName, false);
                bossMusicState = BOSS_MUSIC_STATE_PAUSED;
                break;
            } default: {
                break;
            }
            }
        }
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
    
}


/**
 * @brief Ticks the logic of gameplay-related things.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayState::doGameplayLogic(float deltaT) {

    //Camera movement.
    if(!curLeaderPtr) {
        //If there's no leader being controlled, might as well move the camera.
        Point coords;
        float dummyAngle;
        float dummyMagnitude;
        leaderMovement.getInfo(&coords, &dummyAngle, &dummyMagnitude);
        game.view.cam.targetPos = game.view.cam.pos + (coords * 120.0f / game.view.cam.zoom);
    }
    
    game.view.cam.tick(deltaT);
    game.view.updateTransformations();
    game.view.updateBox();
    game.audio.setCameraPos(
        game.view.box[0] + game.view.boxMargin,
        game.view.box[1] - game.view.boxMargin
    );
    
    if(!msgBox) {
    
        /************************************
        *                              .-.  *
        *   Timer things - gameplay   ( L ) *
        *                              `-´  *
        *************************************/
        
        //Mouse cursor.
        Point mouseCursorSpeed;
        float dummyAngle;
        float dummyMagnitude;
        cursorMovement.getInfo(
            &mouseCursorSpeed, &dummyAngle, &dummyMagnitude
        );
        mouseCursorSpeed =
            mouseCursorSpeed * deltaT* game.options.controls.cursorSpeed;
            
        game.mouseCursor.winPos += mouseCursorSpeed;
        
        game.view.cursorWorldPos = game.mouseCursor.winPos;
        al_transform_coordinates(
            &game.view.windowToWorldTransform,
            &game.view.cursorWorldPos.x, &game.view.cursorWorldPos.y
        );
        
        areaTimePassed += deltaT;
        if(curInterlude == INTERLUDE_NONE) {
            gameplayTimePassed += deltaT;
            dayMinutes +=
                (game.curAreaData->dayTimeSpeed * deltaT / 60.0f);
            if(dayMinutes > 60 * 24) {
                dayMinutes -= 60 * 24;
            }
        }
        
        //Tick all particles.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Logic -- Particles");
        }
        
        particles.tickAll(deltaT);
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Tick all status effect animations.
        for(auto &s : game.content.statusTypes.list) {
            s.second->overlayAnim.tick(deltaT);
        }
        
        /*******************
        *             +--+ *
        *   Sectors   |  | *
        *             +--+ *
        ********************/
        if(game.perfMon) {
            game.perfMon->startMeasurement("Logic -- Sector animation");
        }
        
        for(size_t s = 0; s < game.curAreaData->sectors.size(); s++) {
            Sector* sPtr = game.curAreaData->sectors[s];
            
            if(sPtr->drainingLiquid) {
            
                sPtr->liquidDrainLeft -= deltaT;
                
                if(sPtr->liquidDrainLeft <= 0) {
                
                    if(sPtr->hazard && sPtr->hazard->associatedLiquid) {
                        sPtr->hazard = nullptr;
                        pathMgr.handleSectorHazardChange(sPtr);
                    }
                    
                    sPtr->liquidDrainLeft = 0;
                    sPtr->drainingLiquid = false;
                    
                    unordered_set<Vertex*> sectorVertexes;
                    for(size_t e = 0; e < sPtr->edges.size(); e++) {
                        sectorVertexes.insert(sPtr->edges[e]->vertexes[0]);
                        sectorVertexes.insert(sPtr->edges[e]->vertexes[1]);
                    }
                    updateOffsetEffectCaches(
                        game.liquidLimitEffectCaches,
                        sectorVertexes,
                        doesEdgeHaveLiquidLimit,
                        getLiquidLimitLength,
                        getLiquidLimitColor
                    );
                }
            }
            
            if(sPtr->scroll.x != 0 || sPtr->scroll.y != 0) {
                sPtr->textureInfo.translation += sPtr->scroll * deltaT;
            }
        }
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        
        /*****************
        *                *
        *   Mobs   ()--> *
        *                *
        ******************/
        
        size_t oldNrLivingLeaders = nrLivingLeaders;
        //Some setup to calculate how far the leader walks.
        Leader* oldLeader = curLeaderPtr;
        Point oldLeaderPos;
        bool oldLeaderWasWalking = false;
        if(curLeaderPtr) {
            oldLeaderPos = curLeaderPtr->pos;
            oldLeaderWasWalking =
                curLeaderPtr->active &&
                !hasFlag(
                    curLeaderPtr->chaseInfo.flags,
                    CHASE_FLAG_TELEPORT
                ) &&
                !hasFlag(
                    curLeaderPtr->chaseInfo.flags,
                    CHASE_FLAG_TELEPORTS_CONSTANTLY
                ) &&
                curLeaderPtr->chaseInfo.state == CHASE_STATE_CHASING;
        }
        
        updateAreaActiveCells();
        updateMobIsActiveFlag();
        
        size_t nMobs = mobs.all.size();
        for(size_t m = 0; m < nMobs; m++) {
            //Tick the mob.
            Mob* mPtr = mobs.all[m];
            if(
                !hasFlag(
                    mPtr->type->inactiveLogic,
                    INACTIVE_LOGIC_FLAG_TICKS
                ) && !mPtr->isActive &&
                mPtr->timeAlive > 0.1f
            ) {
                continue;
            }
            
            mPtr->tick(deltaT);
            if(!mPtr->isStoredInsideMob()) {
                processMobInteractions(mPtr, m);
            }
        }
        
        for(size_t m = 0; m < nMobs;) {
            //Mob deletion.
            Mob* mPtr = mobs.all[m];
            if(mPtr->toDelete) {
                deleteMob(mPtr);
                nMobs--;
                continue;
            }
            m++;
        }
        
        doGameplayLeaderLogic(deltaT);
        
        if(
            curLeaderPtr && curLeaderPtr == oldLeader &&
            oldLeaderWasWalking
        ) {
            //This more or less tells us how far the leader walked in this
            //frame. It's not perfect, since it will also count the leader
            //getting pushed and knocked back whilst in the chasing state.
            //It also won't count the movement if the active leader changed
            //midway through.
            //But those are rare cases that don't really affect much in the
            //grand scheme of things, and don't really matter for a fun stat.
            game.statistics.distanceWalked +=
                Distance(oldLeaderPos, curLeaderPtr->pos).toFloat();
        }
        
        nrLivingLeaders = 0;
        for(size_t l = 0; l < mobs.leaders.size(); l++) {
            if(mobs.leaders[l]->health > 0.0f) {
                nrLivingLeaders++;
            }
        }
        if(nrLivingLeaders < oldNrLivingLeaders) {
            game.statistics.leaderKos +=
                oldNrLivingLeaders - nrLivingLeaders;
        }
        leadersKod = startingNrOfLeaders - nrLivingLeaders;
        
        
        /**************************
        *                    /  / *
        *   Precipitation     / / *
        *                   /  /  *
        **************************/
        
        /*
        if(
            curAreaData.weatherCondition.precipitationType !=
            PRECIPITATION_TYPE_NONE
        ) {
            precipitationTimer.tick(deltaT);
            if(precipitationTimer.ticked) {
                precipitationTimer = timer(
                    curAreaData.weatherCondition.
                    precipitationFrequency.getRandomNumber()
                );
                precipitationTimer.start();
                precipitation.pushBack(point(0.0f));
            }
        
            for(size_t p = 0; p < precipitation.size();) {
                precipitation[p].y +=
                    curAreaData.weatherCondition.
                    precipitationSpeed.getRandomNumber() * deltaT;
                if(precipitation[p].y > scrH) {
                    precipitation.erase(precipitation.begin() + p);
                } else {
                    p++;
                }
            }
        }
        */
        
        
        /******************
        *             ___ *
        *   Mission   \ / *
        *              O  *
        *******************/
        if(
            game.curAreaData->type == AREA_TYPE_MISSION &&
            game.curAreaData->mission.goal == MISSION_GOAL_GET_TO_EXIT
        ) {
            curLeadersInMissionExit = 0;
            for(size_t l = 0; l < mobs.leaders.size(); l++) {
                Mob* lPtr = mobs.leaders[l];
                if(
                    !isInContainer(
                        missionRemainingMobIds, mobs.leaders[l]->id
                    )
                ) {
                    //Not a required leader.
                    continue;
                }
                if(
                    fabs(
                        lPtr->pos.x -
                        game.curAreaData->mission.goalExitCenter.x
                    ) <=
                    game.curAreaData->mission.goalExitSize.x / 2.0f &&
                    fabs(
                        lPtr->pos.y -
                        game.curAreaData->mission.goalExitCenter.y
                    ) <=
                    game.curAreaData->mission.goalExitSize.y / 2.0f
                ) {
                    curLeadersInMissionExit++;
                }
            }
        }
        
        float realGoalRatio = 0.0f;
        int goalCurAmount =
            game.missionGoals[game.curAreaData->mission.goal]->getCurAmount(
                this
            );
        int goalReqAmount =
            game.missionGoals[game.curAreaData->mission.goal]->getReqAmount(
                this
            );
        if(goalReqAmount != 0.0f) {
            realGoalRatio = goalCurAmount / (float) goalReqAmount;
        }
        goalIndicatorRatio +=
            (realGoalRatio - goalIndicatorRatio) *
            (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * deltaT);
            
        if(game.curAreaData->mission.failHudPrimaryCond != INVALID) {
            float realFailRatio = 0.0f;
            int failCurAmount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudPrimaryCond
                ]->getCurAmount(this);
            int failReqAmount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudPrimaryCond
                ]->getReqAmount(this);
            if(failReqAmount != 0.0f) {
                realFailRatio = failCurAmount / (float) failReqAmount;
            }
            fail1IndicatorRatio +=
                (realFailRatio - fail1IndicatorRatio) *
                (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * deltaT);
        }
        
        if(game.curAreaData->mission.failHudSecondaryCond != INVALID) {
            float realFailRatio = 0.0f;
            int failCurAmount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudSecondaryCond
                ]->getCurAmount(this);
            int failReqAmount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudSecondaryCond
                ]->getReqAmount(this);
            if(failReqAmount != 0.0f) {
                realFailRatio = failCurAmount / (float) failReqAmount;
            }
            fail2IndicatorRatio +=
                (realFailRatio - fail2IndicatorRatio) *
                (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * deltaT);
        }
        
        if(game.curAreaData->type == AREA_TYPE_MISSION) {
            if(curInterlude == INTERLUDE_NONE) {
                if(isMissionClearMet()) {
                    endMission(true);
                } else if(isMissionFailMet(&missionFailReason)) {
                    endMission(false);
                }
            }
            //Reset the positions of the last mission-end-related things,
            //since if they didn't get used in endMission, then they
            //may be stale from here on.
            lastEnemyDefeatedPos = Point(LARGE_FLOAT);
            lastHurtLeaderPos = Point(LARGE_FLOAT);
            lastPikminBornPos = Point(LARGE_FLOAT);
            lastPikminDeathPos = Point(LARGE_FLOAT);
            lastShipThatGotTreasurePos = Point(LARGE_FLOAT);
            
            missionScore = game.curAreaData->mission.startingPoints;
            for(size_t c = 0; c < game.missionScoreCriteria.size(); c++) {
                if(
                    !hasFlag(
                        game.curAreaData->mission.pointHudData,
                        getIdxBitmask(c)
                    )
                ) {
                    continue;
                }
                MissionScoreCriterion* cPtr =
                    game.missionScoreCriteria[c];
                int cScore =
                    cPtr->getScore(this, &game.curAreaData->mission);
                missionScore += cScore;
            }
            if(missionScore != oldMissionScore) {
                missionScoreCurText->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                );
                oldMissionScore = missionScore;
            }
            
            scoreIndicator +=
                (missionScore - scoreIndicator) *
                (HUD::SCORE_INDICATOR_SMOOTHNESS_MULT * deltaT);
                
            int goalCur =
                game.missionGoals[game.curAreaData->mission.goal]->
                getCurAmount(game.states.gameplay);
            if(goalCur != oldMissionGoalCur) {
                missionGoalCurText->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                );
                oldMissionGoalCur = goalCur;
            }
            
            if(
                game.curAreaData->mission.failHudPrimaryCond !=
                INVALID
            ) {
                size_t cond =
                    game.curAreaData->mission.failHudPrimaryCond;
                int fail1Cur =
                    game.missionFailConds[cond]->getCurAmount(
                        game.states.gameplay
                    );
                if(fail1Cur != oldMissionFail1Cur) {
                    missionFail1CurText->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    oldMissionFail1Cur = fail1Cur;
                }
            }
            if(
                game.curAreaData->mission.failHudSecondaryCond !=
                INVALID
            ) {
                size_t cond =
                    game.curAreaData->mission.failHudSecondaryCond;
                int fail2Cur =
                    game.missionFailConds[cond]->getCurAmount(
                        game.states.gameplay
                    );
                if(fail2Cur != oldMissionFail2Cur) {
                    missionFail2CurText->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    oldMissionFail2Cur = fail2Cur;
                }
            }
            
        }
        
    } else { //Displaying a gameplay message.
    
        msgBox->tick(deltaT);
        if(msgBox->toDelete) {
            startGameplayMessage("", nullptr);
        }
        
    }
    
    replayTimer.tick(deltaT);
    
    if(!readyForInput) {
        readyForInput = true;
        isInputAllowed = true;
    }
    
}


/**
 * @brief Ticks the logic of in-game menu-related things.
 */
void GameplayState::doMenuLogic() {
    if(onionMenu) {
        if(!onionMenu->toDelete) {
            onionMenu->tick(game.deltaT);
        } else {
            delete onionMenu;
            onionMenu = nullptr;
            paused = false;
            game.audio.handleWorldUnpause();
        }
    } else if(pauseMenu) {
        if(!pauseMenu->toDelete) {
            pauseMenu->tick(game.deltaT);
        } else {
            delete pauseMenu;
            pauseMenu = nullptr;
            paused = false;
            game.audio.handleWorldUnpause();
        }
    }
    
    hud->tick(game.deltaT);
    
    //Process and print framerate and system info.
    if(game.showSystemInfo) {
    
        //Make sure that speed changes don't affect the FPS calculation.
        double realDeltaT = game.deltaT;
        if(game.makerTools.changeSpeed) {
            realDeltaT /=
                game.makerTools.changeSpeedSettings[
                    game.makerTools.changeSpeedSettingIdx
                ];
        }
        
        game.framerateHistory.push_back(game.curFrameProcessTime);
        if(game.framerateHistory.size() > GAME::FRAMERATE_HISTORY_SIZE) {
            game.framerateHistory.erase(game.framerateHistory.begin());
        }
        
        game.framerateLastAvgPoint++;
        
        double sampleAvg;
        double sampleAvgCapped;
        
        if(game.framerateLastAvgPoint >= GAME::FRAMERATE_AVG_SAMPLE_SIZE) {
            //Let's get an average, using FRAMERATE_AVG_SAMPLE_SIZE frames.
            //If we can fit a sample of this size using the most recent
            //unsampled frames, then use those. Otherwise, keep using the last
            //block, which starts at framerateLastAvgPoint.
            //This makes it so the average stays the same for a bit of time,
            //so the player can actually read it.
            if(
                game.framerateLastAvgPoint >
                GAME::FRAMERATE_AVG_SAMPLE_SIZE * 2
            ) {
                game.framerateLastAvgPoint =
                    GAME::FRAMERATE_AVG_SAMPLE_SIZE;
            }
            double sampleAvgSum = 0;
            double sampleAvgCappedSum = 0;
            size_t sampleAvgPointCount = 0;
            size_t sampleSize =
                std::min(
                    (size_t) GAME::FRAMERATE_AVG_SAMPLE_SIZE,
                    game.framerateHistory.size()
                );
                
            for(size_t f = 0; f < sampleSize; f++) {
                size_t idx =
                    game.framerateHistory.size() -
                    game.framerateLastAvgPoint + f;
                sampleAvgSum += game.framerateHistory[idx];
                sampleAvgCappedSum +=
                    std::max(
                        game.framerateHistory[idx],
                        (double) (1.0f / game.options.advanced.targetFps)
                    );
                sampleAvgPointCount++;
            }
            
            sampleAvg =
                sampleAvgSum / (float) sampleAvgPointCount;
            sampleAvgCapped =
                sampleAvgCappedSum / (float) sampleAvgPointCount;
                
        } else {
            //If there are fewer than FRAMERATE_AVG_SAMPLE_SIZE frames in
            //the history, the average will change every frame until we get
            //that. This defeats the purpose of a smoothly-updating number,
            //so until that requirement is filled, let's stick to the oldest
            //record.
            sampleAvg = game.framerateHistory[0];
            sampleAvgCapped =
                std::max(
                    game.framerateHistory[0],
                    (double) (1.0f / game.options.advanced.targetFps)
                );
                
        }
        
        string headerStr =
            boxString("", 12) +
            boxString("Now", 12) +
            boxString("Average", 12) +
            boxString("Target", 12);
        string fpsStr =
            boxString("FPS:", 12) +
            boxString(std::to_string(1.0f / realDeltaT), 12) +
            boxString(std::to_string(1.0f / sampleAvgCapped), 12) +
            boxString(i2s(game.options.advanced.targetFps), 12);
        string fpsUncappedStr =
            boxString("FPS uncap.:", 12) +
            boxString(std::to_string(1.0f / game.curFrameProcessTime), 12) +
            boxString(std::to_string(1.0f / sampleAvg), 12) +
            boxString("-", 12);
        string frameTimeStr =
            boxString("Frame time:", 12) +
            boxString(std::to_string(game.curFrameProcessTime), 12) +
            boxString(std::to_string(sampleAvg), 12) +
            boxString(std::to_string(1.0f / game.options.advanced.targetFps), 12);
        string nMobsStr =
            boxString(i2s(mobs.all.size()), 7);
        string nParticlesStr =
            boxString(i2s(particles.getCount()), 7);
        string resolutionStr =
            i2s(game.winW) + "x" + i2s(game.winH);
        string areaVStr =
            game.curAreaData->version.empty() ?
            "-" :
            game.curAreaData->version;
        string areaMakerStr =
            game.curAreaData->maker.empty() ?
            "-" :
            game.curAreaData->maker;
        string gameVStr =
            game.config.general.version.empty() ? "-" : game.config.general.version;
            
        printInfo(
            headerStr +
            "\n" +
            fpsStr +
            "\n" +
            fpsUncappedStr +
            "\n" +
            frameTimeStr +
            "\n"
            "\n"
            "Mobs: " + nMobsStr + " Particles: " + nParticlesStr +
            "\n"
            "Resolution: " + resolutionStr +
            "\n"
            "Area version " + areaVStr + ", by " + areaMakerStr +
            "\n"
            "Pikifen version " + getEngineVersionString() +
            ", game version " + gameVStr,
            1.0f, 1.0f
        );
        
    } else {
        game.framerateLastAvgPoint = 0;
        game.framerateHistory.clear();
    }
    
    //Print info on a mob.
    if(game.makerTools.infoLock) {
        string nameStr =
            boxString(game.makerTools.infoLock->type->name, 26);
        string coordsStr =
            boxString(
                boxString(f2s(game.makerTools.infoLock->pos.x), 8, " ") +
                boxString(f2s(game.makerTools.infoLock->pos.y), 8, " ") +
                boxString(f2s(game.makerTools.infoLock->z), 7),
                23
            );
        string stateHStr =
            (
                game.makerTools.infoLock->fsm.curState ?
                game.makerTools.infoLock->fsm.curState->name :
                "(None!)"
            );
        for(unsigned char p = 0; p < STATE_HISTORY_SIZE; p++) {
            stateHStr +=
                " " + game.makerTools.infoLock->fsm.prevStateNames[p];
        }
        string animStr =
            game.makerTools.infoLock->anim.curAnim ?
            game.makerTools.infoLock->anim.curAnim->name :
            "(None!)";
        string healthStr =
            boxString(
                boxString(f2s(game.makerTools.infoLock->health), 6) +
                " / " +
                boxString(
                    f2s(game.makerTools.infoLock->maxHealth), 6
                ),
                23
            );
        string timerStr =
            f2s(game.makerTools.infoLock->scriptTimer.timeLeft);
        string varsStr;
        if(!game.makerTools.infoLock->vars.empty()) {
            for(
                auto v = game.makerTools.infoLock->vars.begin();
                v != game.makerTools.infoLock->vars.end(); ++v
            ) {
                varsStr += v->first + "=" + v->second + "; ";
            }
            varsStr.erase(varsStr.size() - 2, 2);
        } else {
            varsStr = "(None)";
        }
        
        printInfo(
            "Mob: " + nameStr + "Coords: " + coordsStr + "\n"
            "Last states: " + stateHStr + "\n"
            "Animation: " + animStr + "\n"
            "Health: " + healthStr + " Timer: " + timerStr + "\n"
            "Vars: " + varsStr,
            5.0f, 3.0f
        );
    }
    
    //Print path info.
    if(game.makerTools.infoLock && game.makerTools.pathInfo) {
        if(game.makerTools.infoLock->pathInfo) {
        
            Path* path = game.makerTools.infoLock->pathInfo;
            string resultStr = pathResultToString(path->result);
            
            string stopsStr =
                boxString(i2s(path->curPathStopIdx + 1), 3) +
                "/" +
                boxString(i2s(path->path.size()), 3);
                
            string settingsStr;
            auto flags = path->settings.flags;
            if(hasFlag(flags, PATH_FOLLOW_FLAG_CAN_CONTINUE)) {
                settingsStr += "can continue, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES)) {
                settingsStr += "ignore obstacles, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_FOLLOW_MOB)) {
                settingsStr += "follow mob, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_FAKED_START)) {
                settingsStr += "faked start, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_FAKED_END)) {
                settingsStr += "faked end, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_SCRIPT_USE)) {
                settingsStr += "script, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_LIGHT_LOAD)) {
                settingsStr += "light load, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_AIRBORNE)) {
                settingsStr += "airborne, ";
            }
            if(settingsStr.size() > 2) {
                //Remove the extra comma and space.
                settingsStr.pop_back();
                settingsStr.pop_back();
            } else {
                settingsStr = "none";
            }
            
            string blockStr = pathBlockReasonToString(path->blockReason);
            
            printInfo(
                "Path calculation result: " + resultStr +
                "\n" +
                "Heading to stop " + stopsStr +
                "\n" +
                "Settings: " + settingsStr +
                "\n" +
                "Block reason: " + blockStr,
                5.0f, 3.0f
            );
            
        } else {
        
            printInfo("Mob is not following any path.", 5.0f, 3.0f);
            
        }
    }
    
    //Print mouse coordinates.
    if(game.makerTools.geometryInfo) {
        Sector* mouseSector =
            getSector(game.view.cursorWorldPos, nullptr, true);
            
        string coordsStr =
            boxString(f2s(game.view.cursorWorldPos.x), 6) + " " +
            boxString(f2s(game.view.cursorWorldPos.y), 6);
        string blockmapStr =
            boxString(
                i2s(game.curAreaData->bmap.getCol(game.view.cursorWorldPos.x)),
                5, " "
            ) +
            i2s(game.curAreaData->bmap.getRow(game.view.cursorWorldPos.y));
        string sectorZStr, sectorLightStr, sectorTexStr;
        if(mouseSector) {
            sectorZStr =
                boxString(f2s(mouseSector->z), 6);
            sectorLightStr =
                boxString(i2s(mouseSector->brightness), 3);
            sectorTexStr =
                mouseSector->textureInfo.bmpName;
        }
        
        string str =
            "Mouse coords: " + coordsStr +
            "\n"
            "Blockmap under mouse: " + blockmapStr +
            "\n"
            "Sector under mouse: ";
            
        if(mouseSector) {
            str +=
                "\n"
                "  Z: " + sectorZStr + " Light: " + sectorLightStr +
                "\n"
                "  Texture: " + sectorTexStr;
        } else {
            str += "None";
        }
        
        printInfo(str, 1.0f, 1.0f);
    }
    
    game.makerTools.infoPrintTimer.tick(game.deltaT);
    
    //Big message.
    if(curBigMsg != BIG_MESSAGE_NONE) {
        bigMsgTime += game.deltaT;
    }
    
    switch(curBigMsg) {
    case BIG_MESSAGE_NONE: {
        break;
    } case BIG_MESSAGE_READY: {
        if(bigMsgTime >= GAMEPLAY::BIG_MSG_READY_DUR) {
            curBigMsg = BIG_MESSAGE_GO;
            bigMsgTime = 0.0f;
        }
        break;
    } case BIG_MESSAGE_GO: {
        if(bigMsgTime >= GAMEPLAY::BIG_MSG_GO_DUR) {
            curBigMsg = BIG_MESSAGE_NONE;
        }
        break;
    } case BIG_MESSAGE_MISSION_CLEAR: {
        if(bigMsgTime >= GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) {
            curBigMsg = BIG_MESSAGE_NONE;
        }
        break;
    } case BIG_MESSAGE_MISSION_FAILED: {
        if(bigMsgTime >= GAMEPLAY::BIG_MSG_MISSION_FAILED_DUR) {
            curBigMsg = BIG_MESSAGE_NONE;
        }
        break;
    }
    }
    
    //Interlude.
    if(curInterlude != INTERLUDE_NONE) {
        interludeTime += game.deltaT;
    }
    
    switch(curInterlude) {
    case INTERLUDE_NONE: {
        break;
    } case INTERLUDE_READY: {
        if(interludeTime >= GAMEPLAY::BIG_MSG_READY_DUR) {
            curInterlude = INTERLUDE_NONE;
            deltaTMult = 1.0f;
            hud->gui.startAnimation(
                GUI_MANAGER_ANIM_OUT_TO_IN,
                GAMEPLAY::AREA_INTRO_HUD_MOVE_TIME
            );
            game.audio.setCurrentSong(game.curAreaData->songName);
        }
        break;
    } case INTERLUDE_MISSION_END: {
        if(interludeTime >= GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) {
            curInterlude = INTERLUDE_NONE;
            deltaTMult = 1.0f;
            leave(GAMEPLAY_LEAVE_TARGET_END);
        }
        break;
    }
    }
    
    //Area title fade.
    areaTitleFadeTimer.tick(game.deltaT);
    
    //Fade.
    game.fadeMgr.tick(game.deltaT);
}


/**
 * @brief Checks if the player is close to any living enemy and also if
 * they are close to any living boss.
 *
 * @param nearEnemy If not nullptr, whether they are close to an enemy is
 * returned here.
 * @param nearBoss If not nullptr, whether they are close to a boss is
 * returned here.
 */
void GameplayState::isNearEnemyAndBoss(bool* nearEnemy, bool* nearBoss) {
    bool foundEnemy = false;
    bool foundBoss = false;
    for(size_t e = 0; e < game.states.gameplay->mobs.enemies.size(); e++) {
        Enemy* ePtr = game.states.gameplay->mobs.enemies[e];
        if(ePtr->health <= 0.0f) continue;
        
        Distance d = curLeaderPtr->getDistanceBetween(ePtr);
        
        if(!ePtr->eneType->isBoss) {
            if(d <= GAMEPLAY::ENEMY_MIX_DISTANCE) {
                foundEnemy = true;
            }
        } else {
            if(d <= GAMEPLAY::BOSS_MUSIC_DISTANCE) {
                foundBoss = true;
            }
        }
        
        if(foundEnemy && foundBoss) break;
    }
    
    if(nearEnemy) *nearEnemy = foundEnemy;
    if(nearBoss) *nearBoss = foundBoss;
}


/**
 * @brief Checks if the mission goal has been met.
 *
 * @return Whether the goal is met.
 */
bool GameplayState::isMissionClearMet() {
    return game.missionGoals[game.curAreaData->mission.goal]->isMet(this);
}


/**
 * @brief Checks if a mission fail condition has been met.
 *
 * @param reason The reason gets returned here, if any.
 * @return Whether a failure condition is met.
 */
bool GameplayState::isMissionFailMet(MISSION_FAIL_COND* reason) {
    for(size_t f = 0; f < game.missionFailConds.size(); f++) {
        if(
            hasFlag(
                game.curAreaData->mission.failConditions,
                getIdxBitmask(f)
            )
        ) {
            if(game.missionFailConds[f]->isMet(this)) {
                *reason = (MISSION_FAIL_COND) f;
                return true;
            }
        }
    }
    return false;
}


/**
 * @brief Marks all area cells in a given region as active.
 *
 * @param topLeft Top-left coordinates (in world coordinates)
 * of the region.
 * @param bottomRight Bottom-right coordinates (in world coordinates)
 * of the region.
 */
void GameplayState::markAreaCellsActive(
    const Point &topLeft, const Point &bottomRight
) {
    int fromX =
        (topLeft.x - game.curAreaData->bmap.topLeftCorner.x) /
        GEOMETRY::AREA_CELL_SIZE;
    int toX =
        (bottomRight.x - game.curAreaData->bmap.topLeftCorner.x) /
        GEOMETRY::AREA_CELL_SIZE;
    int fromY =
        (topLeft.y - game.curAreaData->bmap.topLeftCorner.y) /
        GEOMETRY::AREA_CELL_SIZE;
    int toY =
        (bottomRight.y - game.curAreaData->bmap.topLeftCorner.y) /
        GEOMETRY::AREA_CELL_SIZE;
        
    markAreaCellsActive(fromX, toX, fromY, toY);
}


/**
 * @brief Marks all area cells in a given region as active.
 * All coordinates provided are automatically adjusted if out-of-bounds.
 *
 * @param fromX Starting column index of the cells, inclusive.
 * @param toX Ending column index of the cells, inclusive.
 * @param fromY Starting row index of the cells, inclusive.
 * @param toY Ending row index of the cells, inclusive.
 */
void GameplayState::markAreaCellsActive(
    int fromX, int toX, int fromY, int toY
) {
    fromX = std::max(0, fromX);
    toX = std::min(toX, (int) areaActiveCells.size() - 1);
    fromY = std::max(0, fromY);
    toY = std::min(toY, (int) areaActiveCells[0].size() - 1);
    
    for(int x = fromX; x <= toX; x++) {
        for(int y = fromY; y <= toY; y++) {
            areaActiveCells[x][y] = true;
        }
    }
}


/**
 * @brief Handles the logic required to tick a specific mob and its interactions
 * with other mobs.
 *
 * @param mPtr Mob to process.
 * @param m Index of the mob.
 */
void GameplayState::processMobInteractions(Mob* mPtr, size_t m) {
    vector<PendingIntermobEvent> pendingIntermobEvents;
    MobState* stateBefore = mPtr->fsm.curState;
    
    size_t nMobs = mobs.all.size();
    for(size_t m2 = 0; m2 < nMobs; m2++) {
        if(m == m2) continue;
        
        Mob* m2Ptr = mobs.all[m2];
        if(
            !hasFlag(
                m2Ptr->type->inactiveLogic,
                INACTIVE_LOGIC_FLAG_INTERACTIONS
            ) && !m2Ptr->isActive &&
            mPtr->timeAlive > 0.1f
        ) {
            continue;
        }
        if(m2Ptr->toDelete) continue;
        if(m2Ptr->isStoredInsideMob()) continue;
        
        Distance d(mPtr->pos, m2Ptr->pos);
        Distance dBetween = mPtr->getDistanceBetween(m2Ptr, &d);
        
        if(dBetween > mPtr->interactionSpan + m2Ptr->physicalSpan) {
            //The other mob is so far away that there is
            //no interaction possible.
            continue;
        }
        
        if(game.perfMon) {
            game.perfMon->startMeasurement("Objects -- Touching others");
        }
        
        if(d <= mPtr->physicalSpan + m2Ptr->physicalSpan) {
            //Only check if their radii or hitboxes
            //can (theoretically) reach each other.
            processMobTouches(mPtr, m2Ptr, m, m2, d);
            
        }
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
            game.perfMon->startMeasurement("Objects -- Reaches");
        }
        
        if(
            m2Ptr->health != 0 && mPtr->nearReach != INVALID &&
            !m2Ptr->hasInvisibilityStatus
        ) {
            processMobReaches(
                mPtr, m2Ptr, m, m2, dBetween, pendingIntermobEvents
            );
        }
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
            game.perfMon->startMeasurement("Objects -- Misc. interactions");
        }
        
        processMobMiscInteractions(
            mPtr, m2Ptr, m, m2, d, dBetween, pendingIntermobEvents
        );
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
    }
    
    if(game.perfMon) {
        game.perfMon->startMeasurement("Objects -- Interaction results");
    }
    
    //Check the pending inter-mob events.
    sort(
        pendingIntermobEvents.begin(), pendingIntermobEvents.end(),
    [mPtr] (PendingIntermobEvent e1, PendingIntermobEvent e2) -> bool {
        return
        (
            e1.d.toFloat() -
            (mPtr->radius + e1.mobPtr->radius)
        ) < (
            e2.d.toFloat() -
            (mPtr->radius + e2.mobPtr->radius)
        );
    }
    );
    
    for(size_t e = 0; e < pendingIntermobEvents.size(); e++) {
        if(mPtr->fsm.curState != stateBefore) {
            //We can't go on, since the new state might not even have the
            //event, and the reaches could've also changed.
            break;
        }
        if(!pendingIntermobEvents[e].eventPtr) continue;
        pendingIntermobEvents[e].eventPtr->run(
            mPtr, (void*) pendingIntermobEvents[e].mobPtr
        );
        
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
}


/**
 * @brief Handles the logic between mPtr and m2Ptr regarding
 * miscellaneous things.
 *
 * @param mPtr Mob that's being processed.
 * @param m2Ptr Check against this mob.
 * @param m Index of the mob being processed.
 * @param m2 Index of the mob to check against.
 * @param d Distance between the two's centers.
 * @param dBetween Distance between the two.
 * @param pendingIntermobEvents Vector of events to be processed.
 */
void GameplayState::processMobMiscInteractions(
    Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2,
    const Distance &d, const Distance &dBetween,
    vector<PendingIntermobEvent> &pendingIntermobEvents
) {
    //Find a carriable mob to grab.
    MobEvent* ncoEvent =
        mPtr->fsm.getEvent(MOB_EV_NEAR_CARRIABLE_OBJECT);
    if(
        ncoEvent &&
        m2Ptr->carryInfo &&
        !m2Ptr->carryInfo->isFull()
    ) {
        if(dBetween <= taskRange(mPtr)) {
            pendingIntermobEvents.push_back(
                PendingIntermobEvent(dBetween, ncoEvent, m2Ptr)
            );
        }
    }
    
    //Find a tool mob.
    MobEvent* ntoEvent =
        mPtr->fsm.getEvent(MOB_EV_NEAR_TOOL);
    if(
        ntoEvent &&
        typeid(*m2Ptr) == typeid(Tool)
    ) {
        if(dBetween <= taskRange(mPtr)) {
            Tool* tooPtr = (Tool*) m2Ptr;
            if(tooPtr->reserved && tooPtr->reserved != mPtr) {
                //Another Pikmin is already going for it. Ignore it.
            } else {
                pendingIntermobEvents.push_back(
                    PendingIntermobEvent(dBetween, ntoEvent, m2Ptr)
                );
            }
        }
    }
    
    //Find a group task mob.
    MobEvent* ngtoEvent =
        mPtr->fsm.getEvent(MOB_EV_NEAR_GROUP_TASK);
    if(
        ngtoEvent &&
        m2Ptr->health > 0 &&
        typeid(*m2Ptr) == typeid(GroupTask)
    ) {
        if(dBetween <= taskRange(mPtr)) {
            GroupTask* tasPtr = (GroupTask*) m2Ptr;
            GroupTask::GroupTaskSpot* freeSpot = tasPtr->getFreeSpot();
            if(!freeSpot) {
                //There are no free spots here. Ignore it.
            } else {
                pendingIntermobEvents.push_back(
                    PendingIntermobEvent(dBetween, ngtoEvent, m2Ptr)
                );
            }
        }
        
    }
    
    //"Bumped" by the active leader being nearby.
    MobEvent* touchLeEv =
        mPtr->fsm.getEvent(MOB_EV_TOUCHED_ACTIVE_LEADER);
    if(
        touchLeEv &&
        m2Ptr == curLeaderPtr &&
        //Small hack. This way,
        //Pikmin don't get bumped by leaders that are,
        //for instance, lying down.
        m2Ptr->fsm.curState->id == LEADER_STATE_ACTIVE &&
        d <= game.config.pikmin.idleBumpRange
    ) {
        pendingIntermobEvents.push_back(
            PendingIntermobEvent(dBetween, touchLeEv, m2Ptr)
        );
    }
}


/**
 * @brief Handles the logic between mPtr and m2Ptr regarding everything
 * involving one being in the other's reach.
 *
 * @param mPtr Mob that's being processed.
 * @param m2Ptr Check against this mob.
 * @param m Index of the mob being processed.
 * @param m2 Index of the mob to check against.
 * @param dBetween Distance between the two.
 * @param pendingIntermobEvents Vector of events to be processed.
 */
void GameplayState::processMobReaches(
    Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2, const Distance &dBetween,
    vector<PendingIntermobEvent> &pendingIntermobEvents
) {
    //Check reaches.
    MobEvent* obirEv =
        mPtr->fsm.getEvent(MOB_EV_OBJECT_IN_REACH);
    MobEvent* opirEv =
        mPtr->fsm.getEvent(MOB_EV_OPPONENT_IN_REACH);
        
    if(!obirEv && !opirEv) return;
    
    MobType::Reach* rPtr = &mPtr->type->reaches[mPtr->nearReach];
    float angleDiff =
        getAngleSmallestDiff(
            mPtr->angle,
            getAngle(mPtr->pos, m2Ptr->pos)
        );
        
    if(isMobInReach(rPtr, dBetween, angleDiff)) {
        if(obirEv) {
            pendingIntermobEvents.push_back(
                PendingIntermobEvent(
                    dBetween, obirEv, m2Ptr
                )
            );
        }
        if(opirEv && mPtr->canHunt(m2Ptr)) {
            pendingIntermobEvents.push_back(
                PendingIntermobEvent(
                    dBetween, opirEv, m2Ptr
                )
            );
        }
    }
}


/**
 * @brief Handles the logic between mPtr and m2Ptr regarding everything
 * involving one touching the other.
 *
 * @param mPtr Mob that's being processed.
 * @param m2Ptr Check against this mob.
 * @param m Index of the mob being processed.
 * @param m2 Index of the mob to check against.
 * @param d Distance between the two.
 */
void GameplayState::processMobTouches(
    Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2, Distance &d
) {
    //Check if mob 1 should be pushed by mob 2.
    bool bothIdlePikmin =
        mPtr->type->category->id == MOB_CATEGORY_PIKMIN &&
        m2Ptr->type->category->id == MOB_CATEGORY_PIKMIN &&
        (
            ((Pikmin*) mPtr)->fsm.curState->id == PIKMIN_STATE_IDLING ||
            ((Pikmin*) mPtr)->fsm.curState->id == PIKMIN_STATE_IDLING_H
        ) && (
            ((Pikmin*) m2Ptr)->fsm.curState->id == PIKMIN_STATE_IDLING ||
            ((Pikmin*) m2Ptr)->fsm.curState->id == PIKMIN_STATE_IDLING_H
        );
    bool okToPush = true;
    if(
        hasFlag(mPtr->flags, MOB_FLAG_INTANGIBLE) ||
        hasFlag(m2Ptr->flags, MOB_FLAG_INTANGIBLE)
    ) {
        okToPush = false;
    } else if(!mPtr->type->pushable) {
        okToPush = false;
    } else if(hasFlag(mPtr->flags, MOB_FLAG_UNPUSHABLE)) {
        okToPush = false;
    } else if(mPtr->standingOnMob == m2Ptr) {
        okToPush = false;
    }
    
    if(
        okToPush &&
        (m2Ptr->type->pushes || bothIdlePikmin) && (
            (
                m2Ptr->z < mPtr->z + mPtr->height &&
                m2Ptr->z + m2Ptr->height > mPtr->z
            ) || (
                mPtr->height == 0
            ) || (
                m2Ptr->height == 0
            )
        ) && !(
            //If they are both being carried by Pikmin, one of them
            //shouldn't push, otherwise the Pikmin
            //can get stuck in a deadlock.
            mPtr->carryInfo && mPtr->carryInfo->isMoving &&
            m2Ptr->carryInfo && m2Ptr->carryInfo->isMoving &&
            m < m2
        )
    ) {
        float pushAmount = 0;
        float pushAngle = 0;
        
        if(m2Ptr->type->pushesWithHitboxes) {
            //Push with the hitboxes.
            
            Sprite* s2Ptr;
            m2Ptr->getSpriteData(&s2Ptr, nullptr, nullptr);
            
            for(size_t h = 0; h < s2Ptr->hitboxes.size(); h++) {
                Hitbox* hPtr = &s2Ptr->hitboxes[h];
                if(hPtr->type == HITBOX_TYPE_DISABLED) continue;
                Point hPos(
                    m2Ptr->pos.x + (
                        hPtr->pos.x * m2Ptr->angleCos -
                        hPtr->pos.y * m2Ptr->angleSin
                    ),
                    m2Ptr->pos.y + (
                        hPtr->pos.x * m2Ptr->angleSin +
                        hPtr->pos.y * m2Ptr->angleCos
                    )
                );
                //It's more optimized to get the hitbox position here
                //instead of calling hitbox::getCurPos because
                //we already know the sine and cosine, so they don't
                //need to be re-calculated.
                
                Distance hd(mPtr->pos, hPos);
                if(hd < mPtr->radius + hPtr->radius) {
                    float p =
                        fabs(
                            hd.toFloat() - mPtr->radius -
                            hPtr->radius
                        );
                    if(pushAmount == 0 || p > pushAmount) {
                        pushAmount = p;
                        pushAngle = getAngle(hPos, mPtr->pos);
                    }
                }
            }
            
        } else {
            bool xyCollision = false;
            float tempPushAmount = 0;
            float tempPushAngle = 0;
            if(
                mPtr->rectangularDim.x != 0 &&
                m2Ptr->rectangularDim.x != 0
            ) {
                //Rectangle vs rectangle.
                xyCollision =
                    rectanglesIntersect(
                        mPtr->pos, mPtr->rectangularDim, mPtr->angle,
                        m2Ptr->pos, m2Ptr->rectangularDim, m2Ptr->angle,
                        &tempPushAmount, &tempPushAngle
                    );
            } else if(mPtr->rectangularDim.x != 0) {
                //Rectangle vs circle.
                xyCollision =
                    circleIntersectsRectangle(
                        m2Ptr->pos, m2Ptr->radius,
                        mPtr->pos, mPtr->rectangularDim,
                        mPtr->angle, &tempPushAmount, &tempPushAngle
                    );
                tempPushAngle += TAU / 2.0f;
            } else if(m2Ptr->rectangularDim.x != 0) {
                //Circle vs rectangle.
                xyCollision =
                    circleIntersectsRectangle(
                        mPtr->pos, mPtr->radius,
                        m2Ptr->pos, m2Ptr->rectangularDim,
                        m2Ptr->angle, &tempPushAmount, &tempPushAngle
                    );
            } else {
                //Circle vs circle.
                xyCollision =
                    d <= (mPtr->radius + m2Ptr->radius);
                if(xyCollision) {
                    //Only bother calculating if there's a collision.
                    tempPushAmount =
                        fabs(
                            d.toFloat() - mPtr->radius -
                            m2Ptr->radius
                        );
                    tempPushAngle = getAngle(m2Ptr->pos, mPtr->pos);
                }
            }
            
            if(xyCollision) {
                pushAmount = tempPushAmount;
                if(m2Ptr->type->pushesSoftly) {
                    pushAmount =
                        std::min(
                            pushAmount,
                            (float) (MOB::PUSH_SOFTLY_AMOUNT * game.deltaT)
                        );
                }
                pushAngle = tempPushAngle;
                if(bothIdlePikmin) {
                    //Lower the push.
                    //Basically, make PUSH_EXTRA_AMOUNT do all the work.
                    pushAmount = 0.1f;
                    //Deviate the angle slightly. This way, if two Pikmin
                    //are in the same spot, they don't drag each other forever.
                    pushAngle += 0.1f * (m > m2);
                } else if(
                    mPtr->timeAlive < MOB::PUSH_THROTTLE_TIMEOUT ||
                    m2Ptr->timeAlive < MOB::PUSH_THROTTLE_TIMEOUT
                ) {
                    //If either the pushed mob or the pusher mob spawned
                    //recently, then throttle the push. This avoids stuff like
                    //an enemy spoil pushing said enemy with insane force.
                    //Especially if there are multiple spoils.
                    //Setting the amount to 0.1 means it'll only really use the
                    //push provided by MOB_PUSH_EXTRA_AMOUNT.
                    float timeFactor =
                        std::min(mPtr->timeAlive, m2Ptr->timeAlive);
                    pushAmount *=
                        timeFactor /
                        MOB::PUSH_THROTTLE_TIMEOUT *
                        MOB::PUSH_THROTTLE_FACTOR;
                        
                }
            }
        }
        
        //If the mob is inside the other,
        //it needs to be pushed out.
        if((pushAmount / game.deltaT) > mPtr->pushAmount) {
            mPtr->pushAmount = pushAmount / game.deltaT;
            mPtr->pushAngle = pushAngle;
        }
    }
    
    
    //Check touches. This does not use hitboxes,
    //only the object radii (or rectangular width/height).
    MobEvent* touchOpEv =
        mPtr->fsm.getEvent(MOB_EV_TOUCHED_OPPONENT);
    MobEvent* touchObEv =
        mPtr->fsm.getEvent(MOB_EV_TOUCHED_OBJECT);
    if(touchOpEv || touchObEv) {
    
        bool zTouch;
        if(
            mPtr->height == 0 ||
            m2Ptr->height == 0
        ) {
            zTouch = true;
        } else {
            zTouch =
                !(
                    (m2Ptr->z > mPtr->z + mPtr->height) ||
                    (m2Ptr->z + m2Ptr->height < mPtr->z)
                );
        }
        
        bool xyCollision = false;
        if(
            mPtr->rectangularDim.x != 0 &&
            m2Ptr->rectangularDim.x != 0
        ) {
            //Rectangle vs rectangle.
            xyCollision =
                rectanglesIntersect(
                    mPtr->pos, mPtr->rectangularDim, mPtr->angle,
                    m2Ptr->pos, m2Ptr->rectangularDim, m2Ptr->angle
                );
        } else if(mPtr->rectangularDim.x != 0) {
            //Rectangle vs circle.
            xyCollision =
                circleIntersectsRectangle(
                    m2Ptr->pos, m2Ptr->radius,
                    mPtr->pos, mPtr->rectangularDim,
                    mPtr->angle
                );
        } else if(m2Ptr->rectangularDim.x != 0) {
            //Circle vs rectangle.
            xyCollision =
                circleIntersectsRectangle(
                    mPtr->pos, mPtr->radius,
                    m2Ptr->pos, m2Ptr->rectangularDim,
                    m2Ptr->angle
                );
        } else {
            //Circle vs circle.
            xyCollision =
                d <= (mPtr->radius + m2Ptr->radius);
        }
        
        if(
            zTouch && !hasFlag(m2Ptr->flags, MOB_FLAG_INTANGIBLE) &&
            xyCollision
        ) {
            if(touchObEv) {
                touchObEv->run(mPtr, (void*) m2Ptr);
            }
            if(touchOpEv && mPtr->canHunt(m2Ptr)) {
                touchOpEv->run(mPtr, (void*) m2Ptr);
            }
        }
        
    }
    
    //Check hitbox touches.
    MobEvent* hitboxTouchANEv =
        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_A_N);
    MobEvent* hitboxTouchNAEv =
        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
    MobEvent* hitboxTouchNNEv =
        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_N);
    MobEvent* hitboxTouchEatEv =
        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_EAT);
    MobEvent* hitboxTouchHazEv =
        mPtr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
        
    Sprite* s1Ptr;
    mPtr->getSpriteData(&s1Ptr, nullptr, nullptr);
    Sprite* s2Ptr;
    m2Ptr->getSpriteData(&s2Ptr, nullptr, nullptr);
    
    if(
        (
            hitboxTouchANEv || hitboxTouchNAEv || hitboxTouchNNEv ||
            hitboxTouchEatEv
        ) &&
        s1Ptr && s2Ptr &&
        !s1Ptr->hitboxes.empty() && !s2Ptr->hitboxes.empty()
    ) {
    
        bool reportedANEv = false;
        bool reportedNAEv = false;
        bool reportedNNEv = false;
        bool reportedEatEv = false;
        bool reportedHazEv = false;
        
        for(size_t h1 = 0; h1 < s1Ptr->hitboxes.size(); h1++) {
        
            Hitbox* h1Ptr = &s1Ptr->hitboxes[h1];
            if(h1Ptr->type == HITBOX_TYPE_DISABLED) continue;
            
            for(size_t h2 = 0; h2 < s2Ptr->hitboxes.size(); h2++) {
                Hitbox* h2Ptr = &s2Ptr->hitboxes[h2];
                if(h2Ptr->type == HITBOX_TYPE_DISABLED) continue;
                
                //Get the real hitbox locations.
                Point m1HPos =
                    h1Ptr->getCurPos(
                        mPtr->pos, mPtr->angleCos, mPtr->angleSin
                    );
                Point m2HPos =
                    h2Ptr->getCurPos(
                        m2Ptr->pos, m2Ptr->angleCos, m2Ptr->angleSin
                    );
                float m1HZ = mPtr->z + h1Ptr->z;
                float m2HZ = m2Ptr->z + h2Ptr->z;
                
                bool collided = false;
                
                if(
                    (
                        mPtr->holder.m == m2Ptr &&
                        mPtr->holder.hitboxIdx == h2
                    ) || (
                        m2Ptr->holder.m == mPtr &&
                        m2Ptr->holder.hitboxIdx == h1
                    )
                ) {
                    //Mobs held by a hitbox are obviously touching it.
                    collided = true;
                }
                
                if(!collided) {
                    bool zCollision;
                    if(h1Ptr->height == 0 || h2Ptr->height == 0) {
                        zCollision = true;
                    } else {
                        zCollision =
                            !(
                                (m2HZ > m1HZ + h1Ptr->height) ||
                                (m2HZ + h2Ptr->height < m1HZ)
                            );
                    }
                    
                    if(
                        zCollision &&
                        Distance(m1HPos, m2HPos) <
                        (h1Ptr->radius + h2Ptr->radius)
                    ) {
                        collided = true;
                    }
                }
                
                if(!collided) continue;
                
                //Collision confirmed!
                
                if(
                    hitboxTouchANEv && !reportedANEv &&
                    h1Ptr->type == HITBOX_TYPE_ATTACK &&
                    h2Ptr->type == HITBOX_TYPE_NORMAL
                ) {
                    HitboxInteraction evInfo =
                        HitboxInteraction(
                            m2Ptr, h1Ptr, h2Ptr
                        );
                        
                    hitboxTouchANEv->run(
                        mPtr, (void*) &evInfo
                    );
                    reportedANEv = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitboxTouchEatEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_EAT);
                    hitboxTouchHazEv =
                        mPtr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
                    hitboxTouchNAEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                    hitboxTouchNNEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_N);
                }
                
                if(
                    hitboxTouchNNEv && !reportedNNEv &&
                    h1Ptr->type == HITBOX_TYPE_NORMAL &&
                    h2Ptr->type == HITBOX_TYPE_NORMAL
                ) {
                    HitboxInteraction evInfo =
                        HitboxInteraction(
                            m2Ptr, h1Ptr, h2Ptr
                        );
                        
                    hitboxTouchNNEv->run(
                        mPtr, (void*) &evInfo
                    );
                    reportedNNEv = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitboxTouchEatEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_EAT);
                    hitboxTouchHazEv =
                        mPtr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
                    hitboxTouchNAEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                    hitboxTouchANEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_A_N);
                }
                
                if(
                    h1Ptr->type == HITBOX_TYPE_NORMAL &&
                    h2Ptr->type == HITBOX_TYPE_ATTACK
                ) {
                    //Confirmed damage.
                    
                    //Hazard resistance check.
                    if(
                        h2Ptr->hazard &&
                        mPtr->getHazardVulnerability(h2Ptr->hazard).
                        effectMult == 0.0f
                    ) {
                        continue;
                    }
                    
                    //Should this mob even attack this other mob?
                    if(!m2Ptr->canHurt(mPtr)) {
                        continue;
                    }
                }
                
                //Check if m2 is under any status effect
                //that disables attacks.
                bool disableAttackStatus = false;
                for(size_t s = 0; s < m2Ptr->statuses.size(); s++) {
                    if(m2Ptr->statuses[s].type->disablesAttack) {
                        disableAttackStatus = true;
                        break;
                    }
                }
                
                //First, the "touched eat hitbox" event.
                if(
                    hitboxTouchEatEv &&
                    !reportedEatEv &&
                    !disableAttackStatus &&
                    h1Ptr->type == HITBOX_TYPE_NORMAL &&
                    m2Ptr->chompingMobs.size() <
                    m2Ptr->chompMax &&
                    find(
                        m2Ptr->chompBodyParts.begin(),
                        m2Ptr->chompBodyParts.end(),
                        h2Ptr->bodyPartIdx
                    ) !=
                    m2Ptr->chompBodyParts.end()
                ) {
                    hitboxTouchEatEv->run(
                        mPtr,
                        (void*) m2Ptr,
                        (void*) h2Ptr
                    );
                    reportedEatEv = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitboxTouchHazEv =
                        mPtr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
                    hitboxTouchNAEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                }
                
                //"Touched hazard" event.
                if(
                    hitboxTouchHazEv &&
                    !reportedHazEv &&
                    !disableAttackStatus &&
                    h1Ptr->type == HITBOX_TYPE_NORMAL &&
                    h2Ptr->type == HITBOX_TYPE_ATTACK &&
                    h2Ptr->hazard
                ) {
                    HitboxInteraction evInfo =
                        HitboxInteraction(
                            m2Ptr, h1Ptr, h2Ptr
                        );
                    hitboxTouchHazEv->run(
                        mPtr,
                        (void*) h2Ptr->hazard,
                        (void*) &evInfo
                    );
                    reportedHazEv = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitboxTouchNAEv =
                        mPtr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                }
                
                //"Normal hitbox touched attack hitbox" event.
                if(
                    hitboxTouchNAEv &&
                    !reportedNAEv &&
                    !disableAttackStatus &&
                    h1Ptr->type == HITBOX_TYPE_NORMAL &&
                    h2Ptr->type == HITBOX_TYPE_ATTACK
                ) {
                    HitboxInteraction evInfo =
                        HitboxInteraction(
                            m2Ptr, h1Ptr, h2Ptr
                        );
                    hitboxTouchNAEv->run(
                        mPtr, (void*) &evInfo
                    );
                    reportedNAEv = true;
                    
                }
            }
        }
    }
}


/**
 * @brief Updates the grid that represents which area cells are active
 * for this frame.
 */
void GameplayState::updateAreaActiveCells() {
    //Initialize the grid to false.
    for(size_t x = 0; x < areaActiveCells.size(); x++) {
        for(size_t y = 0; y < areaActiveCells[x].size(); y++) {
            areaActiveCells[x][y] = false;
        }
    }
    
    //Mark the 3x3 region around Pikmin and leaders as active.
    for(size_t p = 0; p < mobs.pikmin.size(); p++) {
        markAreaCellsActive(
            mobs.pikmin[p]->pos - GEOMETRY::AREA_CELL_SIZE,
            mobs.pikmin[p]->pos + GEOMETRY::AREA_CELL_SIZE
        );
    }
    
    for(size_t l = 0; l < mobs.leaders.size(); l++) {
        markAreaCellsActive(
            mobs.leaders[l]->pos - GEOMETRY::AREA_CELL_SIZE,
            mobs.leaders[l]->pos + GEOMETRY::AREA_CELL_SIZE
        );
    }
    
    //Mark the region in-camera (plus padding) as active.
    markAreaCellsActive(game.view.box[0], game.view.box[1]);
}


/**
 * @brief Updates the "isActive" member variable of all mobs for this frame.
 */
void GameplayState::updateMobIsActiveFlag() {
    unordered_set<Mob*> childMobs;
    
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* mPtr = mobs.all[m];
        
        int cellX =
            (mPtr->pos.x - game.curAreaData->bmap.topLeftCorner.x) /
            GEOMETRY::AREA_CELL_SIZE;
        int cellY =
            (mPtr->pos.y - game.curAreaData->bmap.topLeftCorner.y) /
            GEOMETRY::AREA_CELL_SIZE;
        if(
            cellX < 0 ||
            cellX >= (int) game.states.gameplay->areaActiveCells.size()
        ) {
            mPtr->isActive = false;
        } else if(
            cellY < 0 ||
            cellY >= (int) game.states.gameplay->areaActiveCells[0].size()
        ) {
            mPtr->isActive = false;
        } else {
            mPtr->isActive =
                game.states.gameplay->areaActiveCells[cellX][cellY];
        }
        
        if(mPtr->parent && mPtr->parent->m) childMobs.insert(mPtr);
    }
    
    for(const auto &m : childMobs) {
        if(m->isActive) m->parent->m->isActive = true;
    }
    
    for(auto &m : childMobs) {
        if(m->parent->m->isActive) m->isActive = true;
    }
}
