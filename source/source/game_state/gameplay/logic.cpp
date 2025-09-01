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
#include "../../content/mob/pile.h"
#include "../../content/mob/resource.h"
#include "../../content/mob/tool.h"
#include "../../core/const.h"
#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"


/**
 * @brief Ticks the logic of aesthetic things regarding the leader.
 * If the game is paused, these can be frozen in place without
 * any negative impact.
 *
 * @param player The player responsible.
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayState::doAestheticLeaderLogic(Player* player, float deltaT) {
    if(!player->leaderPtr) return;
    
    //Swarming arrows.
    if(player->swarmMagnitude) {
        player->leaderPtr->swarmNextArrowTimer.tick(deltaT);
    }
    
    Distance leaderToCursorDist(
        player->leaderPtr->pos, player->leaderCursorWorld
    );
    for(size_t a = 0; a < player->leaderPtr->swarmArrows.size(); ) {
        player->leaderPtr->swarmArrows[a] +=
            GAMEPLAY::SWARM_ARROW_SPEED * deltaT;
            
        Distance maxDist =
            (player->swarmMagnitude > 0) ?
            Distance(game.config.rules.cursorMaxDist * player->swarmMagnitude) :
            leaderToCursorDist;
            
        if(maxDist < player->leaderPtr->swarmArrows[a]) {
            player->leaderPtr->swarmArrows.erase(
                player->leaderPtr->swarmArrows.begin() + a
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
            getAngle(player->leaderPtr->pos, player->leaderCursorWorld);
        whistlePos = angleToCoordinates(whistleAngle, whistleDist);
        whistlePos += player->leaderPtr->pos;
    } else {
        whistleDist = leaderToCursorDist.toFloat();
        whistlePos = player->leaderCursorWorld;
    }
    
    player->whistle.tick(
        deltaT, whistlePos,
        player->leaderPtr->leaType->whistleRange, whistleDist
    );
    
    //Where the cursor is.
    player->cursorHeightDiffLight = 0;
    
    if(leaderToCursorDist > game.config.rules.throwMaxDist) {
        float throwAngle =
            getAngle(player->leaderPtr->pos, player->leaderCursorWorld);
        player->throwDest =
            angleToCoordinates(throwAngle, game.config.rules.throwMaxDist);
        player->throwDest += player->leaderPtr->pos;
    } else {
        player->throwDest = player->leaderCursorWorld;
    }
    
    player->throwDestMob = nullptr;
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* mPtr = mobs.all[m];
        if(!bBoxCheck(player->throwDest, mPtr->pos, mPtr->physicalSpan)) {
            //Too far away; of course the cursor isn't on it.
            continue;
        }
        if(!mPtr->type->pushable && !mPtr->type->walkable) {
            //If it doesn't push and can't be walked on, there's probably
            //nothing really for the Pikmin to land on top of.
            continue;
        }
        if(
            player->throwDestMob &&
            mPtr->z + mPtr->height <
            player->throwDestMob->z + player->throwDestMob->height
        ) {
            //If this mob is lower than the previous known "under cursor" mob,
            //then forget it.
            continue;
        }
        if(!mPtr->isPointOn(player->throwDest)) {
            //The cursor is not really on top of this mob.
            continue;
        }
        
        player->throwDestMob = mPtr;
    }
    
    player->leaderCursorSector =
        getSector(player->leaderCursorWorld, nullptr, true);
        
    player->throwDestSector =
        getSector(player->throwDest, nullptr, true);
        
    if(player->leaderCursorSector) {
        player->cursorHeightDiffLight =
            (player->leaderCursorSector->z - player->leaderPtr->z) * 0.001;
        player->cursorHeightDiffLight =
            std::clamp(player->cursorHeightDiffLight, -0.1f, 0.1f);
    }
    
    //Enemy or treasure points.
    int curLeaderCursorMobPoints = 0;
    if(game.curAreaData->type == AREA_TYPE_MISSION) {
        Mob* mPtr = getEnemyOrTreasureOnCursor(player);
        if(mPtr) {
            bool applicable;
            curLeaderCursorMobPoints = mPtr->getMissionPoints(&applicable);
            if(!applicable) curLeaderCursorMobPoints = 0;
        }
    }
    
    if(curLeaderCursorMobPoints != 0) {
        player->leaderCursorMobPoints = curLeaderCursorMobPoints;
        player->leaderCursorMobPointsAlpha =
            inchTowards(
                player->leaderCursorMobPointsAlpha,
                1.0f,
                DRAWING::CURSOR_MOB_POINTS_ALPHA_SPEED * deltaT
            );
    } else {
        player->leaderCursorMobPointsAlpha =
            inchTowards(
                player->leaderCursorMobPointsAlpha,
                0.0f,
                DRAWING::CURSOR_MOB_POINTS_ALPHA_SPEED * deltaT
            );
    }
    
}


/**
 * @brief Ticks the logic of aesthetic things.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayState::doAestheticLogic(float deltaT) {
    for(Player& player : players) {
        //Leader stuff.
        doAestheticLeaderLogic(&player, deltaT);

        //Camera shake.
        player.view.shaker.tick(deltaT);
    }
    
    //Specific animations.
    game.sysContent.anmSparks.tick(deltaT);
}


/**
 * @brief Ticks the logic of leader gameplay-related things.
 *
 * @param player The player responsible.
 * @param deltaT How long the frame's tick is, in seconds.
 */
void GameplayState::doGameplayLeaderLogic(Player* player, float deltaT) {
    if(!player->leaderPtr) return;
    
    if(game.perfMon) {
        game.perfMon->startMeasurement("Logic -- Current leader");
    }
    
    if(player->leaderPtr->toDelete) {
        game.states.gameplay->updateAvailableLeaders();
        changeToNextLeader(player, true, true, true);
    }
    
    /********************
    *              ***  *
    *   Whistle   * O * *
    *              ***  *
    ********************/
    
    if(
        player->whistle.whistling &&
        player->whistle.radius < player->leaderPtr->leaType->whistleRange
    ) {
        player->whistle.radius += game.config.rules.whistleGrowthSpeed * deltaT;
        if(player->whistle.radius > player->leaderPtr->leaType->whistleRange) {
            player->whistle.radius = player->leaderPtr->leaType->whistleRange;
        }
    }
    
    //Current leader movement.
    Point dummyCoords;
    float dummyAngle;
    float leaderMoveMagnitude;
    player->leaderMovement.getInfo(
        &dummyCoords, &dummyAngle, &leaderMoveMagnitude
    );
    if(leaderMoveMagnitude < 0.75) {
        player->leaderPtr->fsm.runEvent(
            LEADER_EV_MOVE_END, (void*) &player->leaderMovement
        );
    } else {
        player->leaderPtr->fsm.runEvent(
            LEADER_EV_MOVE_START, (void*) &player->leaderMovement
        );
    }
    
    if(interlude.get() == INTERLUDE_NONE) {
        //Adjust the camera position.
        float leaderWeight = 1.0f;
        float cursorWeight = game.options.misc.cursorCamWeight;
        float groupWeight = 0.0f;
        
        Point groupCenter = player->leaderPtr->pos;
        if(!player->leaderPtr->group->members.empty()) {
            Point tl = player->leaderPtr->group->members[0]->pos;
            Point br = tl;
            for(
                size_t m = 1; m < player->leaderPtr->group->members.size(); m++
            ) {
                Mob* member = player->leaderPtr->group->members[m];
                updateMinMaxCoords(tl, br, member->pos);
            }
            groupCenter.x = (tl.x + br.x) / 2.0f;
            groupCenter.y = (tl.y + br.y) / 2.0f;
            groupWeight = 0.1f;
            
            Distance groupDist(player->leaderPtr->pos, groupCenter);
            if(groupDist > 500) {
                //If the group is too far away, limit how far the camera can go.
                float extra = groupDist.toFloat() / 500;
                groupWeight *= (1.0f / extra);
            }
        }
        
        float weightSums = leaderWeight + cursorWeight + groupWeight;
        if(weightSums == 0.0f) weightSums = 0.01f;
        leaderWeight /= weightSums;
        cursorWeight /= weightSums;
        groupWeight /= weightSums;
        
        player->view.cam.targetPos =
            player->leaderPtr->pos * leaderWeight +
            player->leaderCursorWorld * cursorWeight +
            groupCenter * groupWeight;
    }
    
    //Check what to show on the notification, if anything.
    player->notification.setEnabled(false);
    
    bool notificationDone = false;
    
    //Lying down stop notification.
    if(
        !notificationDone &&
        player->leaderPtr->carryInfo
    ) {
        player->notification.setEnabled(true);
        player->notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Get up",
            Point(
                player->leaderPtr->pos.x,
                player->leaderPtr->pos.y - player->leaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    //Get up notification.
    if(
        !notificationDone &&
        player->leaderPtr->fsm.curState->id == LEADER_STATE_KNOCKED_DOWN
    ) {
        player->notification.setEnabled(true);
        player->notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Get up",
            Point(
                player->leaderPtr->pos.x,
                player->leaderPtr->pos.y - player->leaderPtr->radius
            )
        );
        notificationDone = true;
    }
    //Auto-throw stop notification.
    if(
        !notificationDone &&
        player->leaderPtr->autoThrowRepeater.time != LARGE_FLOAT &&
        game.options.controls.autoThrowMode == AUTO_THROW_MODE_TOGGLE
    ) {
        player->notification.setEnabled(true);
        player->notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_THROW).inputSource,
            "Stop throwing",
            Point(
                player->leaderPtr->pos.x,
                player->leaderPtr->pos.y - player->leaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    //Pluck stop notification.
    if(
        !notificationDone &&
        player->leaderPtr->autoPlucking
    ) {
        player->notification.setEnabled(true);
        player->notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Stop",
            Point(
                player->leaderPtr->pos.x,
                player->leaderPtr->pos.y - player->leaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    //Go Here stop notification.
    if(
        !notificationDone &&
        player->leaderPtr->midGoHere
    ) {
        player->notification.setEnabled(true);
        player->notification.setContents(
            game.controls.findBind(PLAYER_ACTION_TYPE_WHISTLE).inputSource,
            "Stop",
            Point(
                player->leaderPtr->pos.x,
                player->leaderPtr->pos.y - player->leaderPtr->radius
            )
        );
        notificationDone = true;
    }
    
    if(!player->leaderPtr->autoPlucking) {
        Distance closestD;
        Distance d;
        
        //Ship healing notification.
        player->closeToShipToHeal = nullptr;
        for(size_t s = 0; s < mobs.ships.size(); s++) {
            Ship* sPtr = mobs.ships[s];
            d = Distance(player->leaderPtr->pos, sPtr->pos);
            if(!sPtr->isLeaderOnCp(player->leaderPtr)) {
                continue;
            }
            if(player->leaderPtr->health == player->leaderPtr->maxHealth) {
                continue;
            }
            if(!sPtr->shiType->canHeal) {
                continue;
            }
            if(d < closestD || !player->closeToShipToHeal) {
                player->closeToShipToHeal = sPtr;
                closestD = d;
                player->notification.setEnabled(true);
                player->notification.setContents(
                    game.controls.findBind(
                        PLAYER_ACTION_TYPE_THROW
                    ).inputSource,
                    "Repair suit",
                    Point(
                        player->closeToShipToHeal->pos.x,
                        player->closeToShipToHeal->pos.y -
                        player->closeToShipToHeal->radius
                    )
                );
                notificationDone = true;
            }
        }
        
        //Interactable mob notification.
        closestD = 0;
        d = 0;
        player->closeToInteractableToUse = nullptr;
        if(!notificationDone) {
            for(size_t i = 0; i < mobs.interactables.size(); i++) {
                d =
                    Distance(
                        player->leaderPtr->pos, mobs.interactables[i]->pos
                    );
                if(d > mobs.interactables[i]->intType->triggerRange) {
                    continue;
                }
                if(d < closestD || !player->closeToInteractableToUse) {
                    player->closeToInteractableToUse = mobs.interactables[i];
                    closestD = d;
                    player->notification.setEnabled(true);
                    player->notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        player->closeToInteractableToUse->intType->promptText,
                        Point(
                            player->closeToInteractableToUse->pos.x,
                            player->closeToInteractableToUse->pos.y -
                            player->closeToInteractableToUse->radius
                        )
                    );
                    notificationDone = true;
                }
            }
        }
        
        //Pikmin pluck notification.
        closestD = 0;
        d = 0;
        player->closeToPikminToPluck = nullptr;
        if(!notificationDone) {
            Pikmin* p = getClosestSprout(player->leaderPtr->pos, &d, false);
            if(p && d <= game.config.leaders.pluckRange) {
                player->closeToPikminToPluck = p;
                player->notification.setEnabled(true);
                player->notification.setContents(
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
        player->closeToNestToOpen = nullptr;
        if(!notificationDone) {
            for(size_t o = 0; o < mobs.onions.size(); o++) {
                d = Distance(player->leaderPtr->pos, mobs.onions[o]->pos);
                if(d > game.config.leaders.onionOpenRange) continue;
                if(d < closestD || !player->closeToNestToOpen) {
                    player->closeToNestToOpen = mobs.onions[o]->nest;
                    closestD = d;
                    player->notification.setEnabled(true);
                    player->notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        "Check",
                        Point(
                            player->closeToNestToOpen->mPtr->pos.x,
                            player->closeToNestToOpen->mPtr->pos.y -
                            player->closeToNestToOpen->mPtr->radius
                        )
                    );
                    notificationDone = true;
                }
            }
            for(size_t s = 0; s < mobs.ships.size(); s++) {
                d = Distance(player->leaderPtr->pos, mobs.ships[s]->pos);
                if(!mobs.ships[s]->isLeaderOnCp(player->leaderPtr)) {
                    continue;
                }
                if(mobs.ships[s]->shiType->nest->pikTypes.empty()) {
                    continue;
                }
                if(d < closestD || !player->closeToNestToOpen) {
                    player->closeToNestToOpen = mobs.ships[s]->nest;
                    closestD = d;
                    player->notification.setEnabled(true);
                    player->notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        "Check",
                        Point(
                            player->closeToNestToOpen->mPtr->pos.x,
                            player->closeToNestToOpen->mPtr->pos.y -
                            player->closeToNestToOpen->mPtr->radius
                        )
                    );
                    notificationDone = true;
                }
            }
        }
    }
    
    player->notification.tick(deltaT);
    
    /********************
    *             .-.   *
    *   Cursor   ( = )> *
    *             `-´   *
    ********************/
    
    Point mouseCursorSpeed;
    float dummyMagnitude;
    player->cursorMovement.getInfo(
        &mouseCursorSpeed, &dummyAngle, &dummyMagnitude
    );
    mouseCursorSpeed =
        mouseCursorSpeed * deltaT * game.options.controls.cursorSpeed;
        
    player->leaderCursorWorld = player->view.cursorWorldPos;
    
    float cursorAngle =
        getAngle(player->leaderPtr->pos, player->leaderCursorWorld);
        
    Distance leaderToCursorDist(
        player->leaderPtr->pos, player->leaderCursorWorld
    );
    if(leaderToCursorDist > game.config.rules.cursorMaxDist) {
        //Cursor goes beyond the range limit.
        player->leaderCursorWorld.x =
            player->leaderPtr->pos.x +
            (cos(cursorAngle) * game.config.rules.cursorMaxDist);
        player->leaderCursorWorld.y =
            player->leaderPtr->pos.y +
            (sin(cursorAngle) * game.config.rules.cursorMaxDist);
            
        if(mouseCursorSpeed.x != 0 || mouseCursorSpeed.y != 0) {
            //If we're speeding the mouse cursor (via analog stick),
            //don't let it go beyond the edges.
            player->view.cursorWorldPos = player->leaderCursorWorld;
            game.mouseCursor.winPos = player->view.cursorWorldPos;
            al_transform_coordinates(
                &player->view.worldToWindowTransform,
                &game.mouseCursor.winPos.x, &game.mouseCursor.winPos.y
            );
        }
    }
    
    player->leaderCursorWin = player->leaderCursorWorld;
    al_transform_coordinates(
        &player->view.worldToWindowTransform,
        &player->leaderCursorWin.x, &player->leaderCursorWin.y
    );
    
    
    /***********************************
    *                             ***  *
    *   Current leader's group   ****O *
    *                             ***  *
    ************************************/
    
    updateClosestGroupMembers(player);
    if(!player->leaderPtr->holding.empty()) {
        player->closestGroupMember[BUBBLE_RELATION_CURRENT] =
            player->leaderPtr->holding[0];
    }
    
    float oldSwarmMagnitude = player->swarmMagnitude;
    Point swarmCoords;
    float newSwarmAngle;
    player->swarmMovement.getInfo(
        &swarmCoords, &newSwarmAngle, &player->swarmMagnitude
    );
    if(player->swarmMagnitude > 0) {
        //This stops arrows that were fading away to the left from
        //turning to angle 0 because the magnitude reached 0.
        player->swarmAngle = newSwarmAngle;
    }
    
    if(player->swarmCursor) {
        player->swarmAngle = cursorAngle;
        leaderToCursorDist =
            Distance(player->leaderPtr->pos, player->leaderCursorWorld);
        player->swarmMagnitude =
            leaderToCursorDist.toFloat() / game.config.rules.cursorMaxDist;
    }
    
    if(oldSwarmMagnitude != player->swarmMagnitude) {
        if(player->swarmMagnitude != 0) {
            player->leaderPtr->signalSwarmStart();
        } else {
            player->leaderPtr->signalSwarmEnd();
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
        interlude.get() == INTERLUDE_NONE
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
                game.audio.setCurrentSong(
                    game.sysContentNames.sngBoss, true, false
                );
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

    for(Player& player : players) {
        //Manual camera movement.
        if(!player.leaderPtr) {
            //If there's no leader being controlled,
            //might as well move the camera.
            Point coords;
            float dummyAngle;
            float dummyMagnitude;
            player.leaderMovement.getInfo(
                &coords, &dummyAngle, &dummyMagnitude
            );
            player.view.cam.targetPos =
                player.view.cam.pos + (coords * 120.0f / player.view.cam.zoom);
        }
        
        player.view.cam.tick(deltaT);
        player.view.updateTransformations();
        player.view.updateBox();
        game.audio.setCameraPos(
            player.view.box[0] + player.view.boxMargin,
            player.view.box[1] - player.view.boxMargin
        );
    }
    
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
        players[0].cursorMovement.getInfo(
            &mouseCursorSpeed, &dummyAngle, &dummyMagnitude
        );
        mouseCursorSpeed =
            mouseCursorSpeed * deltaT * game.options.controls.cursorSpeed;
            
        game.mouseCursor.winPos += mouseCursorSpeed;
        
        game.editorsView.cursorWorldPos = game.mouseCursor.winPos;
        al_transform_coordinates(
            &game.editorsView.windowToWorldTransform,
            &game.editorsView.cursorWorldPos.x,
            &game.editorsView.cursorWorldPos.y
        );
        
        areaTimePassed += deltaT;
        if(interlude.get() == INTERLUDE_NONE) {
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
        for(auto& s : game.content.statusTypes.list) {
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
        
        Leader* oldLeaders[MAX_PLAYERS];
        Point oldLeaderPos[MAX_PLAYERS];
        bool oldLeaderWasWalking[MAX_PLAYERS];
        for(size_t p = 0; p < players.size(); p++) {
            Player* player = &players[p];
            //Some setup to calculate how far the leader walks.
            oldLeaders[p] = player->leaderPtr;
            oldLeaderWasWalking[p] = false;
            if(player->leaderPtr) {
                oldLeaderPos[p] = player->leaderPtr->pos;
                oldLeaderWasWalking[p] =
                    player->leaderPtr->player &&
                    !hasFlag(
                        player->leaderPtr->chaseInfo.flags,
                        CHASE_FLAG_TELEPORT
                    ) &&
                    !hasFlag(
                        player->leaderPtr->chaseInfo.flags,
                        CHASE_FLAG_TELEPORTS_CONSTANTLY
                    ) &&
                    player->leaderPtr->chaseInfo.state == CHASE_STATE_CHASING;
            }
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
        
        for(size_t p = 0; p < players.size(); p++) {
            Player* player = &players[p];
            doGameplayLeaderLogic(player, deltaT);
            
            if(
                player->leaderPtr && player->leaderPtr == oldLeaders[p] &&
                oldLeaderWasWalking[p]
            ) {
                //This more or less tells us how far the leader walked in this
                //frame. It's not perfect, since it will also count the leader
                //getting pushed and knocked back whilst in the chasing state.
                //It also won't count the movement if the active leader changed
                //midway through.
                //But those are rare cases that don't really affect much in the
                //grand scheme of things, and don't really matter
                //for a fun stat.
                game.statistics.distanceWalked +=
                    Distance(oldLeaderPos[p], player->leaderPtr->pos).toFloat();
            }
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
            if(interlude.get() == INTERLUDE_NONE) {
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
                MISSION_MEDAL oldMedal =
                    game.curAreaData->mission.getScoreMedal(oldMissionScore);
                MISSION_MEDAL newMedal =
                    game.curAreaData->mission.getScoreMedal(missionScore);
                if(oldMedal < newMedal) {
                    medalGotItJuiceTimer = 0.0f;
                    game.audio.createUiSoundSource(
                        game.sysContent.sndMedalGotIt,
                    { .volume = 0.50f }
                    );
                }
                
                oldMissionScore = missionScore;
            }
            
            scoreFlapper +=
                (missionScore - scoreFlapper) *
                (HUD::SCORE_INDICATOR_SMOOTHNESS_MULT * deltaT);
                
            medalGotItJuiceTimer += deltaT;
            
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
            
            float timeLimit = 0;
            if(
                hasFlag(
                    game.curAreaData->mission.failConditions,
                    getIdxBitmask(MISSION_FAIL_COND_TIME_LIMIT)
                )
            ) {
                timeLimit = game.curAreaData->mission.failTimeLimit;
            } else if(
                game.curAreaData->mission.goal == MISSION_GOAL_TIMED_SURVIVAL
            ) {
                timeLimit = game.curAreaData->mission.goalAmount;
            }
            
            if(
                timeLimit >= 120.0f &&
                game.states.gameplay->bigMsg.get() == BIG_MESSAGE_NONE
            ) {
                //It makes sense to only show the warning if the mission
                //is long enough to the point where the player could lose
                //track of where the final minute is.
                float timeLeftCurFrame =
                    timeLimit - game.states.gameplay->gameplayTimePassed;
                float timeLeftPrevFrame =
                    timeLeftCurFrame + game.deltaT;
                if(
                    timeLeftPrevFrame > 60.0f && timeLeftCurFrame <= 60.0f
                ) {
                    game.states.gameplay->bigMsg.set(BIG_MESSAGE_ONE_MIN_LEFT);
                    game.audio.createUiSoundSource(
                        game.sysContent.sndOneMinuteLeft,
                    { .volume = 0.5f }
                    );
                }
            }
            
            if(
                timeLimit >= 30.0f &&
                game.states.gameplay->bigMsg.get() == BIG_MESSAGE_NONE
            ) {
                //It makes sense to only tick the countdown if the
                //final ten seconds would be exciting, which isn't the case
                //on short missions.
                float timeLeftCurFrame =
                    timeLimit - game.states.gameplay->gameplayTimePassed;
                float timeLeftPrevFrame =
                    timeLeftCurFrame + game.deltaT;
                if(
                    timeLeftCurFrame <= 10.0f &&
                    timeLeftCurFrame > 0.0f &&
                    floor(timeLeftPrevFrame) > floor(timeLeftCurFrame)
                ) {
                    game.audio.createUiSoundSource(
                        game.sysContent.sndCountdownTick
                    );
                }
            }
            
            
        }
        
    } else {
    
        //Displaying a gameplay message.
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
    
    for(Player& player : players) {
        player.hud->tick(game.deltaT);
    }
    
    //Print info on a mob.
    if(game.makerTools.infoLock) {
        string nameStr =
            resizeString(
                "#" + i2s(game.makerTools.infoLock->id) + " " +
                game.makerTools.infoLock->type->name,
                26
            );
        string coordsStr =
            resizeString(
                resizeString(f2s(game.makerTools.infoLock->pos.x), 8, " ") +
                resizeString(f2s(game.makerTools.infoLock->pos.y), 8, " ") +
                resizeString(f2s(game.makerTools.infoLock->z), 7),
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
            resizeString(
                resizeString(f2s(game.makerTools.infoLock->health), 6) +
                " / " +
                resizeString(f2s(game.makerTools.infoLock->maxHealth), 6),
                23, true, true
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
        
        game.console.write(
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
                resizeString(i2s(path->curPathStopIdx + 1), 3) +
                "/" +
                resizeString(i2s(path->path.size()), 3);
                
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
            
            game.console.write(
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
        
            game.console.write("Mob is not following any path.", 5.0f, 3.0f);
            
        }
    }
    
    //Print mouse coordinates.
    if(game.makerTools.geometryInfo) {
        Sector* mouseSector =
            getSector(players[0].view.cursorWorldPos, nullptr, true);
            
        string coordsStr =
            resizeString(f2s(players[0].view.cursorWorldPos.x), 6) + " " +
            resizeString(f2s(players[0].view.cursorWorldPos.y), 6);
        string blockmapStr =
            resizeString(
                i2s(
                    game.curAreaData->bmap.getCol(
                        players[0].view.cursorWorldPos.x
                    )
                ),
                5
            ) +
            i2s(
                game.curAreaData->bmap.getRow(
                    players[0].view.cursorWorldPos.y
                )
            );
        string sectorZStr, sectorLightStr, sectorTexStr;
        if(mouseSector) {
            sectorZStr =
                resizeString(f2s(mouseSector->z), 6);
            sectorLightStr =
                resizeString(i2s(mouseSector->brightness), 3);
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
        
        game.console.write(str, 1.0f, 1.0f);
    }
    
    //Big message.
    bigMsg.tick(game.deltaT);
    
    switch(bigMsg.get()) {
    case BIG_MESSAGE_NONE: {
        break;
    } case BIG_MESSAGE_READY: {
        if(bigMsg.getTime() >= GAMEPLAY::BIG_MSG_READY_DUR) {
            bigMsg.set(BIG_MESSAGE_GO);
            game.audio.createUiSoundSource(game.sysContent.sndGo);
        }
        break;
    } case BIG_MESSAGE_GO: {
        if(bigMsg.getTime() >= GAMEPLAY::BIG_MSG_GO_DUR) {
            bigMsg.set(BIG_MESSAGE_NONE);
        }
        break;
    } case BIG_MESSAGE_ONE_MIN_LEFT: {
        if(bigMsg.getTime() >= GAMEPLAY::BIG_MSG_ONE_MIN_LEFT_DUR) {
            bigMsg.set(BIG_MESSAGE_NONE);
        }
        break;
    } case BIG_MESSAGE_MISSION_CLEAR: {
        if(bigMsg.getTime() >= GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) {
            bigMsg.set(BIG_MESSAGE_NONE);
        }
        break;
    } case BIG_MESSAGE_MISSION_FAILED: {
        if(bigMsg.getTime() >= GAMEPLAY::BIG_MSG_MISSION_FAILED_DUR) {
            bigMsg.set(BIG_MESSAGE_NONE);
        }
        break;
    }
    }
    
    //Interlude.
    interlude.tick(game.deltaT);
    
    switch(interlude.get()) {
    case INTERLUDE_NONE: {
        break;
    } case INTERLUDE_READY: {
        if(interlude.getTime() >= GAMEPLAY::BIG_MSG_READY_DUR) {
            interlude.set(INTERLUDE_NONE, false);
            deltaTMult = 1.0f;
            for(Player& player : players) {
                player.hud->gui.startAnimation(
                    GUI_MANAGER_ANIM_OUT_TO_IN,
                    GAMEPLAY::AREA_INTRO_HUD_MOVE_TIME
                );
            }
            game.audio.setCurrentSong(game.curAreaData->songName);
        }
        break;
    } case INTERLUDE_MISSION_END: {
        if(interlude.getTime() >= GAMEPLAY::BIG_MSG_MISSION_CLEAR_DUR) {
            interlude.set(INTERLUDE_NONE, false);
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
    for(Player& player : players) {
        if(!player.leaderPtr) continue;
        for(size_t e = 0; e < game.states.gameplay->mobs.enemies.size(); e++) {
            Enemy* ePtr = game.states.gameplay->mobs.enemies[e];
            if(ePtr->health <= 0.0f) continue;
            
            Distance d = player.leaderPtr->getDistanceBetween(ePtr);
            
            if(!ePtr->isBoss) {
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
    }
    
    if(nearEnemy) *nearEnemy = foundEnemy;
    if(nearBoss) *nearBoss = foundBoss;
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
    const Point& topLeft, const Point& bottomRight
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
    const Distance& d, const Distance& dBetween,
    vector<PendingIntermobEvent>& pendingIntermobEvents
) {
    //Find a carriable mob to grab.
    MobEvent* ncoEvent =
        mPtr->fsm.getEvent(MOB_EV_NEAR_CARRIABLE_OBJECT);
    if(
        ncoEvent &&
        m2Ptr->carryInfo &&
        mPtr->type->category->id == MOB_CATEGORY_PIKMIN &&
        !m2Ptr->carryInfo->isFull()
    ) {
        Pikmin* pikPtr = (Pikmin*) mPtr;
        if(dBetween <= pikPtr->getTaskRange()) {
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
        mPtr->type->category->id == MOB_CATEGORY_PIKMIN &&
        m2Ptr->type->category->id == MOB_CATEGORY_TOOLS
    ) {
        Pikmin* pikPtr = (Pikmin*) mPtr;
        if(dBetween <= pikPtr->getTaskRange()) {
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
        mPtr->type->category->id == MOB_CATEGORY_PIKMIN &&
        m2Ptr->type->category->id == MOB_CATEGORY_GROUP_TASKS
    ) {
        Pikmin* pikPtr = (Pikmin*) mPtr;
        if(dBetween <= pikPtr->getTaskRange()) {
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
    
    //"Bumped" by an active leader being nearby.
    MobEvent* touchLeEv =
        mPtr->fsm.getEvent(MOB_EV_TOUCHED_ACTIVE_LEADER);
    if(touchLeEv) {
        for(Player& player : players) {
            if(
                m2Ptr == player.leaderPtr &&
                //Small hack. This way,
                //Pikmin don't get bumped by leaders that are,
                //for instance, lying down.
                m2Ptr->fsm.curState->id == LEADER_STATE_ACTIVE &&
                dBetween <= game.options.misc.pikminBumpDist
            ) {
                pendingIntermobEvents.push_back(
                    PendingIntermobEvent(dBetween, touchLeEv, m2Ptr)
                );
            }
        }
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
    Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2, const Distance& dBetween,
    vector<PendingIntermobEvent>& pendingIntermobEvents
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
    Mob* mPtr, Mob* m2Ptr, size_t m, size_t m2, Distance& d
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
                    isInContainer(m2Ptr->chompBodyParts, h2Ptr->bodyPartIdx)
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
    for(Player& player : players) {
        markAreaCellsActive(player.view.box[0], player.view.box[1]);
    }
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
    
    for(const auto& m : childMobs) {
        if(m->isActive) m->parent->m->isActive = true;
    }
    
    for(auto& m : childMobs) {
        if(m->parent->m->isActive) m->isActive = true;
    }
}
