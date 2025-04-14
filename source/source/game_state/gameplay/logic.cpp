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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void GameplayState::doAestheticLeaderLogic(float delta_t) {
    if(!curLeaderPtr) return;
    
    //Swarming arrows.
    if(swarmMagnitude) {
        curLeaderPtr->swarmNextArrowTimer.tick(delta_t);
    }
    
    Distance leader_to_cursor_dist(curLeaderPtr->pos, leaderCursorW);
    for(size_t a = 0; a < curLeaderPtr->swarmArrows.size(); ) {
        curLeaderPtr->swarmArrows[a] +=
            GAMEPLAY::SWARM_ARROW_SPEED * delta_t;
            
        Distance max_dist =
            (swarmMagnitude > 0) ?
            Distance(game.config.rules.cursorMaxDist * swarmMagnitude) :
            leader_to_cursor_dist;
            
        if(max_dist < curLeaderPtr->swarmArrows[a]) {
            curLeaderPtr->swarmArrows.erase(
                curLeaderPtr->swarmArrows.begin() + a
            );
        } else {
            a++;
        }
    }
    
    //Whistle.
    float whistle_dist;
    Point whistle_pos;
    
    if(leader_to_cursor_dist > game.config.rules.whistleMaxDist) {
        whistle_dist = game.config.rules.whistleMaxDist;
        float whistle_angle =
            getAngle(curLeaderPtr->pos, leaderCursorW);
        whistle_pos = angleToCoordinates(whistle_angle, whistle_dist);
        whistle_pos += curLeaderPtr->pos;
    } else {
        whistle_dist = leader_to_cursor_dist.toFloat();
        whistle_pos = leaderCursorW;
    }
    
    whistle.tick(
        delta_t, whistle_pos,
        curLeaderPtr->leaType->whistleRange, whistle_dist
    );
    
    //Where the cursor is.
    cursorHeightDiffLight = 0;
    
    if(leader_to_cursor_dist > game.config.rules.throwMaxDist) {
        float throw_angle =
            getAngle(curLeaderPtr->pos, leaderCursorW);
        throwDest =
            angleToCoordinates(throw_angle, game.config.rules.throwMaxDist);
        throwDest += curLeaderPtr->pos;
    } else {
        throwDest = leaderCursorW;
    }
    
    throwDestMob = nullptr;
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* m_ptr = mobs.all[m];
        if(!BBoxCheck(throwDest, m_ptr->pos, m_ptr->physicalSpan)) {
            //Too far away; of course the cursor isn't on it.
            continue;
        }
        if(!m_ptr->type->pushable && !m_ptr->type->walkable) {
            //If it doesn't push and can't be walked on, there's probably
            //nothing really for the Pikmin to land on top of.
            continue;
        }
        if(
            throwDestMob &&
            m_ptr->z + m_ptr->height <
            throwDestMob->z + throwDestMob->height
        ) {
            //If this mob is lower than the previous known "under cursor" mob,
            //then forget it.
            continue;
        }
        if(!m_ptr->isPointOn(throwDest)) {
            //The cursor is not really on top of this mob.
            continue;
        }
        
        throwDestMob = m_ptr;
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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void GameplayState::doAestheticLogic(float delta_t) {
    //Leader stuff.
    doAestheticLeaderLogic(delta_t);
    
    //Specific animations.
    game.sysContent.anmSparks.tick(delta_t);
}


/**
 * @brief Ticks the logic of leader gameplay-related things.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void GameplayState::doGameplayLeaderLogic(float delta_t) {
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
        whistle.radius += game.config.rules.whistleGrowthSpeed * delta_t;
        if(whistle.radius > curLeaderPtr->leaType->whistleRange) {
            whistle.radius = curLeaderPtr->leaType->whistleRange;
        }
    }
    
    //Current leader movement.
    Point dummy_coords;
    float dummy_angle;
    float leader_move_magnitude;
    leaderMovement.getInfo(
        &dummy_coords, &dummy_angle, &leader_move_magnitude
    );
    if(leader_move_magnitude < 0.75) {
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
        float leader_weight = 1.0f;
        float cursor_weight = game.options.misc.cursorCamWeight;
        float group_weight = 0.0f;
        
        Point group_center = curLeaderPtr->pos;
        if(!curLeaderPtr->group->members.empty()) {
            Point tl = curLeaderPtr->group->members[0]->pos;
            Point br = tl;
            for(size_t m = 1; m < curLeaderPtr->group->members.size(); m++) {
                Mob* member = curLeaderPtr->group->members[m];
                updateMinMaxCoords(tl, br, member->pos);
            }
            group_center.x = (tl.x + br.x) / 2.0f;
            group_center.y = (tl.y + br.y) / 2.0f;
            group_weight = 0.1f;
        }
        
        float weight_sums = leader_weight + cursor_weight + group_weight;
        if(weight_sums == 0.0f) weight_sums = 0.01f;
        leader_weight /= weight_sums;
        cursor_weight /= weight_sums;
        group_weight /= weight_sums;
        
        game.cam.targetPos =
            curLeaderPtr->pos * leader_weight +
            leaderCursorW * cursor_weight +
            group_center * group_weight;
    }
    
    //Check what to show on the notification, if anything.
    notification.setEnabled(false);
    
    bool notification_done = false;
    
    //Lying down stop notification.
    if(
        !notification_done &&
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
        notification_done = true;
    }
    
    //Get up notification.
    if(
        !notification_done &&
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
        notification_done = true;
    }
    //Auto-throw stop notification.
    if(
        !notification_done &&
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
        notification_done = true;
    }
    
    //Pluck stop notification.
    if(
        !notification_done &&
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
        notification_done = true;
    }
    
    //Go Here stop notification.
    if(
        !notification_done &&
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
        notification_done = true;
    }
    
    if(!curLeaderPtr->autoPlucking) {
        Distance closest_d;
        Distance d;
        
        //Ship healing notification.
        closeToShipToHeal = nullptr;
        for(size_t s = 0; s < mobs.ships.size(); s++) {
            Ship* s_ptr = mobs.ships[s];
            d = Distance(curLeaderPtr->pos, s_ptr->pos);
            if(!s_ptr->isLeaderOnCp(curLeaderPtr)) {
                continue;
            }
            if(curLeaderPtr->health == curLeaderPtr->maxHealth) {
                continue;
            }
            if(!s_ptr->shiType->canHeal) {
                continue;
            }
            if(d < closest_d || !closeToShipToHeal) {
                closeToShipToHeal = s_ptr;
                closest_d = d;
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
                notification_done = true;
            }
        }
        
        //Interactable mob notification.
        closest_d = 0;
        d = 0;
        closeToInteractableToUse = nullptr;
        if(!notification_done) {
            for(size_t i = 0; i < mobs.interactables.size(); i++) {
                d = Distance(curLeaderPtr->pos, mobs.interactables[i]->pos);
                if(d > mobs.interactables[i]->intType->triggerRange) {
                    continue;
                }
                if(d < closest_d || !closeToInteractableToUse) {
                    closeToInteractableToUse = mobs.interactables[i];
                    closest_d = d;
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
                    notification_done = true;
                }
            }
        }
        
        //Pikmin pluck notification.
        closest_d = 0;
        d = 0;
        closeToPikminToPluck = nullptr;
        if(!notification_done) {
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
                notification_done = true;
            }
        }
        
        //Nest open notification.
        closest_d = 0;
        d = 0;
        closeToNestToOpen = nullptr;
        if(!notification_done) {
            for(size_t o = 0; o < mobs.onions.size(); o++) {
                d = Distance(curLeaderPtr->pos, mobs.onions[o]->pos);
                if(d > game.config.leaders.onionOpenRange) continue;
                if(d < closest_d || !closeToNestToOpen) {
                    closeToNestToOpen = mobs.onions[o]->nest;
                    closest_d = d;
                    notification.setEnabled(true);
                    notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        "Check",
                        Point(
                            closeToNestToOpen->m_ptr->pos.x,
                            closeToNestToOpen->m_ptr->pos.y -
                            closeToNestToOpen->m_ptr->radius
                        )
                    );
                    notification_done = true;
                }
            }
            for(size_t s = 0; s < mobs.ships.size(); s++) {
                d = Distance(curLeaderPtr->pos, mobs.ships[s]->pos);
                if(!mobs.ships[s]->isLeaderOnCp(curLeaderPtr)) {
                    continue;
                }
                if(mobs.ships[s]->shiType->nest->pik_types.empty()) {
                    continue;
                }
                if(d < closest_d || !closeToNestToOpen) {
                    closeToNestToOpen = mobs.ships[s]->nest;
                    closest_d = d;
                    notification.setEnabled(true);
                    notification.setContents(
                        game.controls.findBind(PLAYER_ACTION_TYPE_THROW).
                        inputSource,
                        "Check",
                        Point(
                            closeToNestToOpen->m_ptr->pos.x,
                            closeToNestToOpen->m_ptr->pos.y -
                            closeToNestToOpen->m_ptr->radius
                        )
                    );
                    notification_done = true;
                }
            }
        }
    }
    
    notification.tick(delta_t);
    
    /********************
    *             .-.   *
    *   Cursor   ( = )> *
    *             `-´   *
    ********************/
    
    Point mouse_cursor_speed;
    float dummy_magnitude;
    cursorMovement.getInfo(
        &mouse_cursor_speed, &dummy_angle, &dummy_magnitude
    );
    mouse_cursor_speed =
        mouse_cursor_speed * delta_t* game.options.controls.cursorSpeed;
        
    leaderCursorW = game.mouseCursor.wPos;
    
    float cursor_angle = getAngle(curLeaderPtr->pos, leaderCursorW);
    
    Distance leader_to_cursor_dist(curLeaderPtr->pos, leaderCursorW);
    if(leader_to_cursor_dist > game.config.rules.cursorMaxDist) {
        //Cursor goes beyond the range limit.
        leaderCursorW.x =
            curLeaderPtr->pos.x +
            (cos(cursor_angle) * game.config.rules.cursorMaxDist);
        leaderCursorW.y =
            curLeaderPtr->pos.y +
            (sin(cursor_angle) * game.config.rules.cursorMaxDist);
            
        if(mouse_cursor_speed.x != 0 || mouse_cursor_speed.y != 0) {
            //If we're speeding the mouse cursor (via analog stick),
            //don't let it go beyond the edges.
            game.mouseCursor.wPos = leaderCursorW;
            game.mouseCursor.sPos = game.mouseCursor.wPos;
            al_transform_coordinates(
                &game.worldToScreenTransform,
                &game.mouseCursor.sPos.x, &game.mouseCursor.sPos.y
            );
        }
    }
    
    leaderCursorS = leaderCursorW;
    al_transform_coordinates(
        &game.worldToScreenTransform,
        &leaderCursorS.x, &leaderCursorS.y
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
    
    float old_swarm_magnitude = swarmMagnitude;
    Point swarm_coords;
    float new_swarm_angle;
    swarmMovement.getInfo(
        &swarm_coords, &new_swarm_angle, &swarmMagnitude
    );
    if(swarmMagnitude > 0) {
        //This stops arrows that were fading away to the left from
        //turning to angle 0 because the magnitude reached 0.
        swarmAngle = new_swarm_angle;
    }
    
    if(swarmCursor) {
        swarmAngle = cursor_angle;
        leader_to_cursor_dist = Distance(curLeaderPtr->pos, leaderCursorW);
        swarmMagnitude =
            leader_to_cursor_dist.toFloat() / game.config.rules.cursorMaxDist;
    }
    
    if(old_swarm_magnitude != swarmMagnitude) {
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
        bool near_enemy = false;
        bool near_boss = false;
        isNearEnemyAndBoss(&near_enemy, &near_boss);
        
        if(near_enemy) {
            game.audio.markMixTrackStatus(MIX_TRACK_TYPE_ENEMY);
        }
        
        if(near_boss) {
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
 * @param delta_t How long the frame's tick is, in seconds.
 */
void GameplayState::doGameplayLogic(float delta_t) {

    //Camera movement.
    if(!curLeaderPtr) {
        //If there's no leader being controlled, might as well move the camera.
        Point coords;
        float dummy_angle;
        float dummy_magnitude;
        leaderMovement.getInfo(&coords, &dummy_angle, &dummy_magnitude);
        game.cam.targetPos = game.cam.pos + (coords * 120.0f / game.cam.zoom);
    }
    
    game.cam.tick(delta_t);
    
    updateTransformations();
    
    game.cam.updateBox();
    
    if(!msgBox) {
    
        /************************************
        *                              .-.  *
        *   Timer things - gameplay   ( L ) *
        *                              `-´  *
        *************************************/
        
        //Mouse cursor.
        Point mouse_cursor_speed;
        float dummy_angle;
        float dummy_magnitude;
        cursorMovement.getInfo(
            &mouse_cursor_speed, &dummy_angle, &dummy_magnitude
        );
        mouse_cursor_speed =
            mouse_cursor_speed * delta_t* game.options.controls.cursorSpeed;
            
        game.mouseCursor.sPos += mouse_cursor_speed;
        
        game.mouseCursor.wPos = game.mouseCursor.sPos;
        al_transform_coordinates(
            &game.screenToWorldTransform,
            &game.mouseCursor.wPos.x, &game.mouseCursor.wPos.y
        );
        
        areaTimePassed += delta_t;
        if(curInterlude == INTERLUDE_NONE) {
            gameplayTimePassed += delta_t;
            dayMinutes +=
                (game.curAreaData->dayTimeSpeed * delta_t / 60.0f);
            if(dayMinutes > 60 * 24) {
                dayMinutes -= 60 * 24;
            }
        }
        
        //Tick all particles.
        if(game.perfMon) {
            game.perfMon->startMeasurement("Logic -- Particles");
        }
        
        particles.tickAll(delta_t);
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
        }
        
        //Tick all status effect animations.
        for(auto &s : game.content.statusTypes.list) {
            s.second->overlayAnim.tick(delta_t);
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
            Sector* s_ptr = game.curAreaData->sectors[s];
            
            if(s_ptr->drainingLiquid) {
            
                s_ptr->liquidDrainLeft -= delta_t;
                
                if(s_ptr->liquidDrainLeft <= 0) {
                
                    for(size_t h = 0; h < s_ptr->hazards.size();) {
                        if(s_ptr->hazards[h]->associatedLiquid) {
                            s_ptr->hazards.erase(s_ptr->hazards.begin() + h);
                            pathMgr.handleSectorHazardChange(s_ptr);
                        } else {
                            h++;
                        }
                    }
                    
                    s_ptr->liquidDrainLeft = 0;
                    s_ptr->drainingLiquid = false;
                    
                    unordered_set<Vertex*> sector_vertexes;
                    for(size_t e = 0; e < s_ptr->edges.size(); e++) {
                        sector_vertexes.insert(s_ptr->edges[e]->vertexes[0]);
                        sector_vertexes.insert(s_ptr->edges[e]->vertexes[1]);
                    }
                    updateOffsetEffectCaches(
                        game.liquidLimitEffectCaches,
                        sector_vertexes,
                        doesEdgeHaveLiquidLimit,
                        getLiquidLimitLength,
                        getLiquidLimitColor
                    );
                }
            }
            
            if(s_ptr->scroll.x != 0 || s_ptr->scroll.y != 0) {
                s_ptr->textureInfo.translation += s_ptr->scroll * delta_t;
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
        
        size_t old_nr_living_leaders = nrLivingLeaders;
        //Some setup to calculate how far the leader walks.
        Leader* old_leader = curLeaderPtr;
        Point old_leader_pos;
        bool old_leader_was_walking = false;
        if(curLeaderPtr) {
            old_leader_pos = curLeaderPtr->pos;
            old_leader_was_walking =
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
        
        size_t n_mobs = mobs.all.size();
        for(size_t m = 0; m < n_mobs; m++) {
            //Tick the mob.
            Mob* m_ptr = mobs.all[m];
            if(
                !hasFlag(
                    m_ptr->type->inactiveLogic,
                    INACTIVE_LOGIC_FLAG_TICKS
                ) && !m_ptr->isActive &&
                m_ptr->timeAlive > 0.1f
            ) {
                continue;
            }
            
            m_ptr->tick(delta_t);
            if(!m_ptr->isStoredInsideMob()) {
                processMobInteractions(m_ptr, m);
            }
        }
        
        for(size_t m = 0; m < n_mobs;) {
            //Mob deletion.
            Mob* m_ptr = mobs.all[m];
            if(m_ptr->toDelete) {
                deleteMob(m_ptr);
                n_mobs--;
                continue;
            }
            m++;
        }
        
        doGameplayLeaderLogic(delta_t);
        
        if(
            curLeaderPtr && curLeaderPtr == old_leader &&
            old_leader_was_walking
        ) {
            //This more or less tells us how far the leader walked in this
            //frame. It's not perfect, since it will also count the leader
            //getting pushed and knocked back whilst in the chasing state.
            //It also won't count the movement if the active leader changed
            //midway through.
            //But those are rare cases that don't really affect much in the
            //grand scheme of things, and don't really matter for a fun stat.
            game.statistics.distanceWalked +=
                Distance(old_leader_pos, curLeaderPtr->pos).toFloat();
        }
        
        nrLivingLeaders = 0;
        for(size_t l = 0; l < mobs.leaders.size(); l++) {
            if(mobs.leaders[l]->health > 0.0f) {
                nrLivingLeaders++;
            }
        }
        if(nrLivingLeaders < old_nr_living_leaders) {
            game.statistics.leaderKos +=
                old_nr_living_leaders - nrLivingLeaders;
        }
        leadersKod = startingNrOfLeaders - nrLivingLeaders;
        
        
        /**************************
        *                    /  / *
        *   Precipitation     / / *
        *                   /  /  *
        **************************/
        
        /*
        if(
            curAreaData.weather_condition.precipitation_type !=
            PRECIPITATION_TYPE_NONE
        ) {
            precipitationTimer.tick(delta_t);
            if(precipitationTimer.ticked) {
                precipitationTimer = timer(
                    curAreaData.weather_condition.
                    precipitation_frequency.get_random_number()
                );
                precipitationTimer.start();
                precipitation.push_back(point(0.0f));
            }
        
            for(size_t p = 0; p < precipitation.size();) {
                precipitation[p].y +=
                    curAreaData.weather_condition.
                    precipitation_speed.get_random_number() * delta_t;
                if(precipitation[p].y > scr_h) {
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
                Mob* l_ptr = mobs.leaders[l];
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
                        l_ptr->pos.x -
                        game.curAreaData->mission.goalExitCenter.x
                    ) <=
                    game.curAreaData->mission.goalExitSize.x / 2.0f &&
                    fabs(
                        l_ptr->pos.y -
                        game.curAreaData->mission.goalExitCenter.y
                    ) <=
                    game.curAreaData->mission.goalExitSize.y / 2.0f
                ) {
                    curLeadersInMissionExit++;
                }
            }
        }
        
        float real_goal_ratio = 0.0f;
        int goal_cur_amount =
            game.missionGoals[game.curAreaData->mission.goal]->getCurAmount(
                this
            );
        int goal_req_amount =
            game.missionGoals[game.curAreaData->mission.goal]->getReqAmount(
                this
            );
        if(goal_req_amount != 0.0f) {
            real_goal_ratio = goal_cur_amount / (float) goal_req_amount;
        }
        goalIndicatorRatio +=
            (real_goal_ratio - goalIndicatorRatio) *
            (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * delta_t);
            
        if(game.curAreaData->mission.failHudPrimaryCond != INVALID) {
            float real_fail_ratio = 0.0f;
            int fail_cur_amount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudPrimaryCond
                ]->getCurAmount(this);
            int fail_req_amount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudPrimaryCond
                ]->getReqAmount(this);
            if(fail_req_amount != 0.0f) {
                real_fail_ratio = fail_cur_amount / (float) fail_req_amount;
            }
            fail1IndicatorRatio +=
                (real_fail_ratio - fail1IndicatorRatio) *
                (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * delta_t);
        }
        
        if(game.curAreaData->mission.failHudSecondaryCond != INVALID) {
            float real_fail_ratio = 0.0f;
            int fail_cur_amount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudSecondaryCond
                ]->getCurAmount(this);
            int fail_req_amount =
                game.missionFailConds[
                    game.curAreaData->mission.failHudSecondaryCond
                ]->getReqAmount(this);
            if(fail_req_amount != 0.0f) {
                real_fail_ratio = fail_cur_amount / (float) fail_req_amount;
            }
            fail2IndicatorRatio +=
                (real_fail_ratio - fail2IndicatorRatio) *
                (HUD::GOAL_INDICATOR_SMOOTHNESS_MULT * delta_t);
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
            lastEnemyKilledPos = Point(LARGE_FLOAT);
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
                MissionScoreCriterion* c_ptr =
                    game.missionScoreCriteria[c];
                int c_score =
                    c_ptr->getScore(this, &game.curAreaData->mission);
                missionScore += c_score;
            }
            if(missionScore != oldMissionScore) {
                missionScoreCurText->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                );
                oldMissionScore = missionScore;
            }
            
            scoreIndicator +=
                (missionScore - scoreIndicator) *
                (HUD::SCORE_INDICATOR_SMOOTHNESS_MULT * delta_t);
                
            int goal_cur =
                game.missionGoals[game.curAreaData->mission.goal]->
                getCurAmount(game.states.gameplay);
            if(goal_cur != oldMissionGoalCur) {
                missionGoalCurText->startJuiceAnimation(
                    GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                );
                oldMissionGoalCur = goal_cur;
            }
            
            if(
                game.curAreaData->mission.failHudPrimaryCond !=
                INVALID
            ) {
                size_t cond =
                    game.curAreaData->mission.failHudPrimaryCond;
                int fail_1_cur =
                    game.missionFailConds[cond]->getCurAmount(
                        game.states.gameplay
                    );
                if(fail_1_cur != oldMissionFail1Cur) {
                    missionFail1CurText->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    oldMissionFail1Cur = fail_1_cur;
                }
            }
            if(
                game.curAreaData->mission.failHudSecondaryCond !=
                INVALID
            ) {
                size_t cond =
                    game.curAreaData->mission.failHudSecondaryCond;
                int fail_2_cur =
                    game.missionFailConds[cond]->getCurAmount(
                        game.states.gameplay
                    );
                if(fail_2_cur != oldMissionFail2Cur) {
                    missionFail2CurText->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_HIGH
                    );
                    oldMissionFail2Cur = fail_2_cur;
                }
            }
            
        }
        
    } else { //Displaying a gameplay message.
    
        msgBox->tick(delta_t);
        if(msgBox->toDelete) {
            startGameplayMessage("", nullptr);
        }
        
    }
    
    replayTimer.tick(delta_t);
    
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
        double real_delta_t = game.deltaT;
        if(game.makerTools.changeSpeed) {
            real_delta_t /=
                game.makerTools.changeSpeedSettings[
                    game.makerTools.changeSpeedSettingIdx
                ];
        }
        
        game.framerateHistory.push_back(game.curFrameProcessTime);
        if(game.framerateHistory.size() > GAME::FRAMERATE_HISTORY_SIZE) {
            game.framerateHistory.erase(game.framerateHistory.begin());
        }
        
        game.framerateLastAvgPoint++;
        
        double sample_avg;
        double sample_avg_capped;
        
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
            double sample_avg_sum = 0;
            double sample_avg_capped_sum = 0;
            size_t sample_avg_point_count = 0;
            size_t sample_size =
                std::min(
                    (size_t) GAME::FRAMERATE_AVG_SAMPLE_SIZE,
                    game.framerateHistory.size()
                );
                
            for(size_t f = 0; f < sample_size; f++) {
                size_t idx =
                    game.framerateHistory.size() -
                    game.framerateLastAvgPoint + f;
                sample_avg_sum += game.framerateHistory[idx];
                sample_avg_capped_sum +=
                    std::max(
                        game.framerateHistory[idx],
                        (double) (1.0f / game.options.advanced.targetFps)
                    );
                sample_avg_point_count++;
            }
            
            sample_avg =
                sample_avg_sum / (float) sample_avg_point_count;
            sample_avg_capped =
                sample_avg_capped_sum / (float) sample_avg_point_count;
                
        } else {
            //If there are fewer than FRAMERATE_AVG_SAMPLE_SIZE frames in
            //the history, the average will change every frame until we get
            //that. This defeats the purpose of a smoothly-updating number,
            //so until that requirement is filled, let's stick to the oldest
            //record.
            sample_avg = game.framerateHistory[0];
            sample_avg_capped =
                std::max(
                    game.framerateHistory[0],
                    (double) (1.0f / game.options.advanced.targetFps)
                );
                
        }
        
        string header_str =
            boxString("", 12) +
            boxString("Now", 12) +
            boxString("Average", 12) +
            boxString("Target", 12);
        string fps_str =
            boxString("FPS:", 12) +
            boxString(std::to_string(1.0f / real_delta_t), 12) +
            boxString(std::to_string(1.0f / sample_avg_capped), 12) +
            boxString(i2s(game.options.advanced.targetFps), 12);
        string fps_uncapped_str =
            boxString("FPS uncap.:", 12) +
            boxString(std::to_string(1.0f / game.curFrameProcessTime), 12) +
            boxString(std::to_string(1.0f / sample_avg), 12) +
            boxString("-", 12);
        string frame_time_str =
            boxString("Frame time:", 12) +
            boxString(std::to_string(game.curFrameProcessTime), 12) +
            boxString(std::to_string(sample_avg), 12) +
            boxString(std::to_string(1.0f / game.options.advanced.targetFps), 12);
        string n_mobs_str =
            boxString(i2s(mobs.all.size()), 7);
        string n_particles_str =
            boxString(i2s(particles.getCount()), 7);
        string resolution_str =
            i2s(game.winW) + "x" + i2s(game.winH);
        string area_v_str =
            game.curAreaData->version.empty() ?
            "-" :
            game.curAreaData->version;
        string area_maker_str =
            game.curAreaData->maker.empty() ?
            "-" :
            game.curAreaData->maker;
        string game_v_str =
            game.config.general.version.empty() ? "-" : game.config.general.version;
            
        printInfo(
            header_str +
            "\n" +
            fps_str +
            "\n" +
            fps_uncapped_str +
            "\n" +
            frame_time_str +
            "\n"
            "\n"
            "Mobs: " + n_mobs_str + " Particles: " + n_particles_str +
            "\n"
            "Resolution: " + resolution_str +
            "\n"
            "Area version " + area_v_str + ", by " + area_maker_str +
            "\n"
            "Pikifen version " + getEngineVersionString() +
            ", game version " + game_v_str,
            1.0f, 1.0f
        );
        
    } else {
        game.framerateLastAvgPoint = 0;
        game.framerateHistory.clear();
    }
    
    //Print info on a mob.
    if(game.makerTools.infoLock) {
        string name_str =
            boxString(game.makerTools.infoLock->type->name, 26);
        string coords_str =
            boxString(
                boxString(f2s(game.makerTools.infoLock->pos.x), 8, " ") +
                boxString(f2s(game.makerTools.infoLock->pos.y), 8, " ") +
                boxString(f2s(game.makerTools.infoLock->z), 7),
                23
            );
        string state_h_str =
            (
                game.makerTools.infoLock->fsm.curState ?
                game.makerTools.infoLock->fsm.curState->name :
                "(None!)"
            );
        for(unsigned char p = 0; p < STATE_HISTORY_SIZE; p++) {
            state_h_str +=
                " " + game.makerTools.infoLock->fsm.prevStateNames[p];
        }
        string anim_str =
            game.makerTools.infoLock->anim.curAnim ?
            game.makerTools.infoLock->anim.curAnim->name :
            "(None!)";
        string health_str =
            boxString(
                boxString(f2s(game.makerTools.infoLock->health), 6) +
                " / " +
                boxString(
                    f2s(game.makerTools.infoLock->maxHealth), 6
                ),
                23
            );
        string timer_str =
            f2s(game.makerTools.infoLock->scriptTimer.timeLeft);
        string vars_str;
        if(!game.makerTools.infoLock->vars.empty()) {
            for(
                auto v = game.makerTools.infoLock->vars.begin();
                v != game.makerTools.infoLock->vars.end(); ++v
            ) {
                vars_str += v->first + "=" + v->second + "; ";
            }
            vars_str.erase(vars_str.size() - 2, 2);
        } else {
            vars_str = "(None)";
        }
        
        printInfo(
            "Mob: " + name_str +
            "Coords: " + coords_str +
            "\n"
            "Last states: " + state_h_str +
            "\n"
            "Animation: " + anim_str +
            "\n"
            "Health: " + health_str + " Timer: " + timer_str +
            "\n"
            "Vars: " + vars_str,
            5.0f, 3.0f
        );
    }
    
    //Print path info.
    if(game.makerTools.infoLock && game.makerTools.pathInfo) {
        if(game.makerTools.infoLock->pathInfo) {
        
            Path* path = game.makerTools.infoLock->pathInfo;
            string result_str = pathResultToString(path->result);
            
            string stops_str =
                boxString(i2s(path->cur_path_stop_idx + 1), 3) +
                "/" +
                boxString(i2s(path->path.size()), 3);
                
            string settings_str;
            auto flags = path->settings.flags;
            if(hasFlag(flags, PATH_FOLLOW_FLAG_CAN_CONTINUE)) {
                settings_str += "can continue, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_IGNORE_OBSTACLES)) {
                settings_str += "ignore obstacles, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_FOLLOW_MOB)) {
                settings_str += "follow mob, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_FAKED_START)) {
                settings_str += "faked start, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_FAKED_END)) {
                settings_str += "faked end, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_SCRIPT_USE)) {
                settings_str += "script, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_LIGHT_LOAD)) {
                settings_str += "light load, ";
            }
            if(hasFlag(flags, PATH_FOLLOW_FLAG_AIRBORNE)) {
                settings_str += "airborne, ";
            }
            if(settings_str.size() > 2) {
                //Remove the extra comma and space.
                settings_str.pop_back();
                settings_str.pop_back();
            } else {
                settings_str = "none";
            }
            
            string block_str = pathBlockReasonToString(path->block_reason);
            
            printInfo(
                "Path calculation result: " + result_str +
                "\n" +
                "Heading to stop " + stops_str +
                "\n" +
                "Settings: " + settings_str +
                "\n" +
                "Block reason: " + block_str,
                5.0f, 3.0f
            );
            
        } else {
        
            printInfo("Mob is not following any path.", 5.0f, 3.0f);
            
        }
    }
    
    //Print mouse coordinates.
    if(game.makerTools.geometryInfo) {
        Sector* mouse_sector =
            getSector(game.mouseCursor.wPos, nullptr, true);
            
        string coords_str =
            boxString(f2s(game.mouseCursor.wPos.x), 6) + " " +
            boxString(f2s(game.mouseCursor.wPos.y), 6);
        string blockmap_str =
            boxString(
                i2s(game.curAreaData->bmap.getCol(game.mouseCursor.wPos.x)),
                5, " "
            ) +
            i2s(game.curAreaData->bmap.getRow(game.mouseCursor.wPos.y));
        string sector_z_str, sector_light_str, sector_tex_str;
        if(mouse_sector) {
            sector_z_str =
                boxString(f2s(mouse_sector->z), 6);
            sector_light_str =
                boxString(i2s(mouse_sector->brightness), 3);
            sector_tex_str =
                mouse_sector->textureInfo.bmpName;
        }
        
        string str =
            "Mouse coords: " + coords_str +
            "\n"
            "Blockmap under mouse: " + blockmap_str +
            "\n"
            "Sector under mouse: ";
            
        if(mouse_sector) {
            str +=
                "\n"
                "  Z: " + sector_z_str + " Light: " + sector_light_str +
                "\n"
                "  Texture: " + sector_tex_str;
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
 * @param near_enemy If not nullptr, whether they are close to an enemy is
 * returned here.
 * @param near_boss If not nullptr, whether they are close to a boss is
 * returned here.
 */
void GameplayState::isNearEnemyAndBoss(bool* near_enemy, bool* near_boss) {
    bool found_enemy = false;
    bool found_boss = false;
    for(size_t e = 0; e < game.states.gameplay->mobs.enemies.size(); e++) {
        Enemy* e_ptr = game.states.gameplay->mobs.enemies[e];
        if(e_ptr->health <= 0.0f) continue;
        
        Distance d = curLeaderPtr->getDistanceBetween(e_ptr);
        
        if(!e_ptr->eneType->isBoss) {
            if(d <= GAMEPLAY::ENEMY_MIX_DISTANCE) {
                found_enemy = true;
            }
        } else {
            if(d <= GAMEPLAY::BOSS_MUSIC_DISTANCE) {
                found_boss = true;
            }
        }
        
        if(found_enemy && found_boss) break;
    }
    
    if(near_enemy) *near_enemy = found_enemy;
    if(near_boss) *near_boss = found_boss;
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
 * @param top_left Top-left coordinates (in world coordinates)
 * of the region.
 * @param bottom_right Bottom-right coordinates (in world coordinates)
 * of the region.
 */
void GameplayState::markAreaCellsActive(
    const Point &top_left, const Point &bottom_right
) {
    int from_x =
        (top_left.x - game.curAreaData->bmap.topLeftCorner.x) /
        GEOMETRY::AREA_CELL_SIZE;
    int to_x =
        (bottom_right.x - game.curAreaData->bmap.topLeftCorner.x) /
        GEOMETRY::AREA_CELL_SIZE;
    int from_y =
        (top_left.y - game.curAreaData->bmap.topLeftCorner.y) /
        GEOMETRY::AREA_CELL_SIZE;
    int to_y =
        (bottom_right.y - game.curAreaData->bmap.topLeftCorner.y) /
        GEOMETRY::AREA_CELL_SIZE;
        
    markAreaCellsActive(from_x, to_x, from_y, to_y);
}


/**
 * @brief Marks all area cells in a given region as active.
 * All coordinates provided are automatically adjusted if out-of-bounds.
 *
 * @param from_x Starting column index of the cells, inclusive.
 * @param to_x Ending column index of the cells, inclusive.
 * @param from_y Starting row index of the cells, inclusive.
 * @param to_y Ending row index of the cells, inclusive.
 */
void GameplayState::markAreaCellsActive(
    int from_x, int to_x, int from_y, int to_y
) {
    from_x = std::max(0, from_x);
    to_x = std::min(to_x, (int) areaActiveCells.size() - 1);
    from_y = std::max(0, from_y);
    to_y = std::min(to_y, (int) areaActiveCells[0].size() - 1);
    
    for(int x = from_x; x <= to_x; x++) {
        for(int y = from_y; y <= to_y; y++) {
            areaActiveCells[x][y] = true;
        }
    }
}


/**
 * @brief Handles the logic required to tick a specific mob and its interactions
 * with other mobs.
 *
 * @param m_ptr Mob to process.
 * @param m Index of the mob.
 */
void GameplayState::processMobInteractions(Mob* m_ptr, size_t m) {
    vector<PendingIntermobEvent> pending_intermob_events;
    MobState* state_before = m_ptr->fsm.curState;
    
    size_t n_mobs = mobs.all.size();
    for(size_t m2 = 0; m2 < n_mobs; m2++) {
        if(m == m2) continue;
        
        Mob* m2_ptr = mobs.all[m2];
        if(
            !hasFlag(
                m2_ptr->type->inactiveLogic,
                INACTIVE_LOGIC_FLAG_INTERACTIONS
            ) && !m2_ptr->isActive &&
            m_ptr->timeAlive > 0.1f
        ) {
            continue;
        }
        if(m2_ptr->toDelete) continue;
        if(m2_ptr->isStoredInsideMob()) continue;
        
        Distance d(m_ptr->pos, m2_ptr->pos);
        Distance d_between = m_ptr->getDistanceBetween(m2_ptr, &d);
        
        if(d_between > m_ptr->interactionSpan + m2_ptr->physicalSpan) {
            //The other mob is so far away that there is
            //no interaction possible.
            continue;
        }
        
        if(game.perfMon) {
            game.perfMon->startMeasurement("Objects -- Touching others");
        }
        
        if(d <= m_ptr->physicalSpan + m2_ptr->physicalSpan) {
            //Only check if their radii or hitboxes
            //can (theoretically) reach each other.
            processMobTouches(m_ptr, m2_ptr, m, m2, d);
            
        }
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
            game.perfMon->startMeasurement("Objects -- Reaches");
        }
        
        if(
            m2_ptr->health != 0 && m_ptr->nearReach != INVALID &&
            !m2_ptr->hasInvisibilityStatus
        ) {
            processMobReaches(
                m_ptr, m2_ptr, m, m2, d_between, pending_intermob_events
            );
        }
        
        if(game.perfMon) {
            game.perfMon->finishMeasurement();
            game.perfMon->startMeasurement("Objects -- Misc. interactions");
        }
        
        processMobMiscInteractions(
            m_ptr, m2_ptr, m, m2, d, d_between, pending_intermob_events
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
        pending_intermob_events.begin(), pending_intermob_events.end(),
    [m_ptr] (PendingIntermobEvent e1, PendingIntermobEvent e2) -> bool {
        return
        (
            e1.d.toFloat() -
            (m_ptr->radius + e1.mobPtr->radius)
        ) < (
            e2.d.toFloat() -
            (m_ptr->radius + e2.mobPtr->radius)
        );
    }
    );
    
    for(size_t e = 0; e < pending_intermob_events.size(); e++) {
        if(m_ptr->fsm.curState != state_before) {
            //We can't go on, since the new state might not even have the
            //event, and the reaches could've also changed.
            break;
        }
        if(!pending_intermob_events[e].eventPtr) continue;
        pending_intermob_events[e].eventPtr->run(
            m_ptr, (void*) pending_intermob_events[e].mobPtr
        );
        
    }
    
    if(game.perfMon) {
        game.perfMon->finishMeasurement();
    }
}


/**
 * @brief Handles the logic between m_ptr and m2_ptr regarding
 * miscellaneous things.
 *
 * @param m_ptr Mob that's being processed.
 * @param m2_ptr Check against this mob.
 * @param m Index of the mob being processed.
 * @param m2 Index of the mob to check against.
 * @param d Distance between the two's centers.
 * @param d_between Distance between the two.
 * @param pending_intermob_events Vector of events to be processed.
 */
void GameplayState::processMobMiscInteractions(
    Mob* m_ptr, Mob* m2_ptr, size_t m, size_t m2,
    const Distance &d, const Distance &d_between,
    vector<PendingIntermobEvent> &pending_intermob_events
) {
    //Find a carriable mob to grab.
    MobEvent* nco_event =
        m_ptr->fsm.getEvent(MOB_EV_NEAR_CARRIABLE_OBJECT);
    if(
        nco_event &&
        m2_ptr->carryInfo &&
        !m2_ptr->carryInfo->isFull()
    ) {
        if(d_between <= taskRange(m_ptr)) {
            pending_intermob_events.push_back(
                PendingIntermobEvent(d_between, nco_event, m2_ptr)
            );
        }
    }
    
    //Find a tool mob.
    MobEvent* nto_event =
        m_ptr->fsm.getEvent(MOB_EV_NEAR_TOOL);
    if(
        nto_event &&
        typeid(*m2_ptr) == typeid(Tool)
    ) {
        if(d_between <= taskRange(m_ptr)) {
            Tool* too_ptr = (Tool*) m2_ptr;
            if(too_ptr->reserved && too_ptr->reserved != m_ptr) {
                //Another Pikmin is already going for it. Ignore it.
            } else {
                pending_intermob_events.push_back(
                    PendingIntermobEvent(d_between, nto_event, m2_ptr)
                );
            }
        }
    }
    
    //Find a group task mob.
    MobEvent* ngto_event =
        m_ptr->fsm.getEvent(MOB_EV_NEAR_GROUP_TASK);
    if(
        ngto_event &&
        m2_ptr->health > 0 &&
        typeid(*m2_ptr) == typeid(GroupTask)
    ) {
        if(d_between <= taskRange(m_ptr)) {
            GroupTask* tas_ptr = (GroupTask*) m2_ptr;
            GroupTask::GroupTaskSpot* free_spot = tas_ptr->getFreeSpot();
            if(!free_spot) {
                //There are no free spots here. Ignore it.
            } else {
                pending_intermob_events.push_back(
                    PendingIntermobEvent(d_between, ngto_event, m2_ptr)
                );
            }
        }
        
    }
    
    //"Bumped" by the active leader being nearby.
    MobEvent* touch_le_ev =
        m_ptr->fsm.getEvent(MOB_EV_TOUCHED_ACTIVE_LEADER);
    if(
        touch_le_ev &&
        m2_ptr == curLeaderPtr &&
        //Small hack. This way,
        //Pikmin don't get bumped by leaders that are,
        //for instance, lying down.
        m2_ptr->fsm.curState->id == LEADER_STATE_ACTIVE &&
        d <= game.config.pikmin.idleBumpRange
    ) {
        pending_intermob_events.push_back(
            PendingIntermobEvent(d_between, touch_le_ev, m2_ptr)
        );
    }
}


/**
 * @brief Handles the logic between m_ptr and m2_ptr regarding everything
 * involving one being in the other's reach.
 *
 * @param m_ptr Mob that's being processed.
 * @param m2_ptr Check against this mob.
 * @param m Index of the mob being processed.
 * @param m2 Index of the mob to check against.
 * @param d_between Distance between the two.
 * @param pending_intermob_events Vector of events to be processed.
 */
void GameplayState::processMobReaches(
    Mob* m_ptr, Mob* m2_ptr, size_t m, size_t m2, const Distance &d_between,
    vector<PendingIntermobEvent> &pending_intermob_events
) {
    //Check reaches.
    MobEvent* obir_ev =
        m_ptr->fsm.getEvent(MOB_EV_OBJECT_IN_REACH);
    MobEvent* opir_ev =
        m_ptr->fsm.getEvent(MOB_EV_OPPONENT_IN_REACH);
        
    if(!obir_ev && !opir_ev) return;
    
    MobType::Reach* r_ptr = &m_ptr->type->reaches[m_ptr->nearReach];
    float angle_diff =
        getAngleSmallestDiff(
            m_ptr->angle,
            getAngle(m_ptr->pos, m2_ptr->pos)
        );
        
    if(isMobInReach(r_ptr, d_between, angle_diff)) {
        if(obir_ev) {
            pending_intermob_events.push_back(
                PendingIntermobEvent(
                    d_between, obir_ev, m2_ptr
                )
            );
        }
        if(opir_ev && m_ptr->canHunt(m2_ptr)) {
            pending_intermob_events.push_back(
                PendingIntermobEvent(
                    d_between, opir_ev, m2_ptr
                )
            );
        }
    }
}


/**
 * @brief Handles the logic between m_ptr and m2_ptr regarding everything
 * involving one touching the other.
 *
 * @param m_ptr Mob that's being processed.
 * @param m2_ptr Check against this mob.
 * @param m Index of the mob being processed.
 * @param m2 Index of the mob to check against.
 * @param d Distance between the two.
 */
void GameplayState::processMobTouches(
    Mob* m_ptr, Mob* m2_ptr, size_t m, size_t m2, Distance &d
) {
    //Check if mob 1 should be pushed by mob 2.
    bool both_idle_pikmin =
        m_ptr->type->category->id == MOB_CATEGORY_PIKMIN &&
        m2_ptr->type->category->id == MOB_CATEGORY_PIKMIN &&
        (
            ((Pikmin*) m_ptr)->fsm.curState->id == PIKMIN_STATE_IDLING ||
            ((Pikmin*) m_ptr)->fsm.curState->id == PIKMIN_STATE_IDLING_H
        ) && (
            ((Pikmin*) m2_ptr)->fsm.curState->id == PIKMIN_STATE_IDLING ||
            ((Pikmin*) m2_ptr)->fsm.curState->id == PIKMIN_STATE_IDLING_H
        );
    bool ok_to_push = true;
    if(
        hasFlag(m_ptr->flags, MOB_FLAG_INTANGIBLE) ||
        hasFlag(m2_ptr->flags, MOB_FLAG_INTANGIBLE)
    ) {
        ok_to_push = false;
    } else if(!m_ptr->type->pushable) {
        ok_to_push = false;
    } else if(hasFlag(m_ptr->flags, MOB_FLAG_UNPUSHABLE)) {
        ok_to_push = false;
    } else if(m_ptr->standingOnMob == m2_ptr) {
        ok_to_push = false;
    }
    
    if(
        ok_to_push &&
        (m2_ptr->type->pushes || both_idle_pikmin) && (
            (
                m2_ptr->z < m_ptr->z + m_ptr->height &&
                m2_ptr->z + m2_ptr->height > m_ptr->z
            ) || (
                m_ptr->height == 0
            ) || (
                m2_ptr->height == 0
            )
        ) && !(
            //If they are both being carried by Pikmin, one of them
            //shouldn't push, otherwise the Pikmin
            //can get stuck in a deadlock.
            m_ptr->carryInfo && m_ptr->carryInfo->isMoving &&
            m2_ptr->carryInfo && m2_ptr->carryInfo->isMoving &&
            m < m2
        )
    ) {
        float push_amount = 0;
        float push_angle = 0;
        
        if(m2_ptr->type->pushesWithHitboxes) {
            //Push with the hitboxes.
            
            Sprite* s2_ptr;
            m2_ptr->getSpriteData(&s2_ptr, nullptr, nullptr);
            
            for(size_t h = 0; h < s2_ptr->hitboxes.size(); h++) {
                Hitbox* h_ptr = &s2_ptr->hitboxes[h];
                if(h_ptr->type == HITBOX_TYPE_DISABLED) continue;
                Point h_pos(
                    m2_ptr->pos.x + (
                        h_ptr->pos.x * m2_ptr->angleCos -
                        h_ptr->pos.y * m2_ptr->angleSin
                    ),
                    m2_ptr->pos.y + (
                        h_ptr->pos.x * m2_ptr->angleSin +
                        h_ptr->pos.y * m2_ptr->angleCos
                    )
                );
                //It's more optimized to get the hitbox position here
                //instead of calling hitbox::getCurPos because
                //we already know the sine and cosine, so they don't
                //need to be re-calculated.
                
                Distance hd(m_ptr->pos, h_pos);
                if(hd < m_ptr->radius + h_ptr->radius) {
                    float p =
                        fabs(
                            hd.toFloat() - m_ptr->radius -
                            h_ptr->radius
                        );
                    if(push_amount == 0 || p > push_amount) {
                        push_amount = p;
                        push_angle = getAngle(h_pos, m_ptr->pos);
                    }
                }
            }
            
        } else {
            bool xy_collision = false;
            float temp_push_amount = 0;
            float temp_push_angle = 0;
            if(
                m_ptr->rectangularDim.x != 0 &&
                m2_ptr->rectangularDim.x != 0
            ) {
                //Rectangle vs rectangle.
                xy_collision =
                    rectanglesIntersect(
                        m_ptr->pos, m_ptr->rectangularDim, m_ptr->angle,
                        m2_ptr->pos, m2_ptr->rectangularDim, m2_ptr->angle,
                        &temp_push_amount, &temp_push_angle
                    );
            } else if(m_ptr->rectangularDim.x != 0) {
                //Rectangle vs circle.
                xy_collision =
                    circleIntersectsRectangle(
                        m2_ptr->pos, m2_ptr->radius,
                        m_ptr->pos, m_ptr->rectangularDim,
                        m_ptr->angle, &temp_push_amount, &temp_push_angle
                    );
                temp_push_angle += TAU / 2.0f;
            } else if(m2_ptr->rectangularDim.x != 0) {
                //Circle vs rectangle.
                xy_collision =
                    circleIntersectsRectangle(
                        m_ptr->pos, m_ptr->radius,
                        m2_ptr->pos, m2_ptr->rectangularDim,
                        m2_ptr->angle, &temp_push_amount, &temp_push_angle
                    );
            } else {
                //Circle vs circle.
                xy_collision =
                    d <= (m_ptr->radius + m2_ptr->radius);
                if(xy_collision) {
                    //Only bother calculating if there's a collision.
                    temp_push_amount =
                        fabs(
                            d.toFloat() - m_ptr->radius -
                            m2_ptr->radius
                        );
                    temp_push_angle = getAngle(m2_ptr->pos, m_ptr->pos);
                }
            }
            
            if(xy_collision) {
                push_amount = temp_push_amount;
                if(m2_ptr->type->pushesSoftly) {
                    push_amount =
                        std::min(
                            push_amount,
                            (float) (MOB::PUSH_SOFTLY_AMOUNT * game.deltaT)
                        );
                }
                push_angle = temp_push_angle;
                if(both_idle_pikmin) {
                    //Lower the push.
                    //Basically, make PUSH_EXTRA_AMOUNT do all the work.
                    push_amount = 0.1f;
                    //Deviate the angle slightly. This way, if two Pikmin
                    //are in the same spot, they don't drag each other forever.
                    push_angle += 0.1f * (m > m2);
                } else if(
                    m_ptr->timeAlive < MOB::PUSH_THROTTLE_TIMEOUT ||
                    m2_ptr->timeAlive < MOB::PUSH_THROTTLE_TIMEOUT
                ) {
                    //If either the pushed mob or the pusher mob spawned
                    //recently, then throttle the push. This avoids stuff like
                    //an enemy spoil pushing said enemy with insane force.
                    //Especially if there are multiple spoils.
                    //Setting the amount to 0.1 means it'll only really use the
                    //push provided by MOB_PUSH_EXTRA_AMOUNT.
                    float time_factor =
                        std::min(m_ptr->timeAlive, m2_ptr->timeAlive);
                    push_amount *=
                        time_factor /
                        MOB::PUSH_THROTTLE_TIMEOUT *
                        MOB::PUSH_THROTTLE_FACTOR;
                        
                }
            }
        }
        
        //If the mob is inside the other,
        //it needs to be pushed out.
        if((push_amount / game.deltaT) > m_ptr->pushAmount) {
            m_ptr->pushAmount = push_amount / game.deltaT;
            m_ptr->pushAngle = push_angle;
        }
    }
    
    
    //Check touches. This does not use hitboxes,
    //only the object radii (or rectangular width/height).
    MobEvent* touch_op_ev =
        m_ptr->fsm.getEvent(MOB_EV_TOUCHED_OPPONENT);
    MobEvent* touch_ob_ev =
        m_ptr->fsm.getEvent(MOB_EV_TOUCHED_OBJECT);
    if(touch_op_ev || touch_ob_ev) {
    
        bool z_touch;
        if(
            m_ptr->height == 0 ||
            m2_ptr->height == 0
        ) {
            z_touch = true;
        } else {
            z_touch =
                !(
                    (m2_ptr->z > m_ptr->z + m_ptr->height) ||
                    (m2_ptr->z + m2_ptr->height < m_ptr->z)
                );
        }
        
        bool xy_collision = false;
        if(
            m_ptr->rectangularDim.x != 0 &&
            m2_ptr->rectangularDim.x != 0
        ) {
            //Rectangle vs rectangle.
            xy_collision =
                rectanglesIntersect(
                    m_ptr->pos, m_ptr->rectangularDim, m_ptr->angle,
                    m2_ptr->pos, m2_ptr->rectangularDim, m2_ptr->angle
                );
        } else if(m_ptr->rectangularDim.x != 0) {
            //Rectangle vs circle.
            xy_collision =
                circleIntersectsRectangle(
                    m2_ptr->pos, m2_ptr->radius,
                    m_ptr->pos, m_ptr->rectangularDim,
                    m_ptr->angle
                );
        } else if(m2_ptr->rectangularDim.x != 0) {
            //Circle vs rectangle.
            xy_collision =
                circleIntersectsRectangle(
                    m_ptr->pos, m_ptr->radius,
                    m2_ptr->pos, m2_ptr->rectangularDim,
                    m2_ptr->angle
                );
        } else {
            //Circle vs circle.
            xy_collision =
                d <= (m_ptr->radius + m2_ptr->radius);
        }
        
        if(
            z_touch && !hasFlag(m2_ptr->flags, MOB_FLAG_INTANGIBLE) &&
            xy_collision
        ) {
            if(touch_ob_ev) {
                touch_ob_ev->run(m_ptr, (void*) m2_ptr);
            }
            if(touch_op_ev && m_ptr->canHunt(m2_ptr)) {
                touch_op_ev->run(m_ptr, (void*) m2_ptr);
            }
        }
        
    }
    
    //Check hitbox touches.
    MobEvent* hitbox_touch_an_ev =
        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_A_N);
    MobEvent* hitbox_touch_na_ev =
        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
    MobEvent* hitbox_touch_nn_ev =
        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_N);
    MobEvent* hitbox_touch_eat_ev =
        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_EAT);
    MobEvent* hitbox_touch_haz_ev =
        m_ptr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
        
    Sprite* s1_ptr;
    m_ptr->getSpriteData(&s1_ptr, nullptr, nullptr);
    Sprite* s2_ptr;
    m2_ptr->getSpriteData(&s2_ptr, nullptr, nullptr);
    
    if(
        (
            hitbox_touch_an_ev || hitbox_touch_na_ev || hitbox_touch_nn_ev ||
            hitbox_touch_eat_ev
        ) &&
        s1_ptr && s2_ptr &&
        !s1_ptr->hitboxes.empty() && !s2_ptr->hitboxes.empty()
    ) {
    
        bool reported_an_ev = false;
        bool reported_na_ev = false;
        bool reported_nn_ev = false;
        bool reported_eat_ev = false;
        bool reported_haz_ev = false;
        
        for(size_t h1 = 0; h1 < s1_ptr->hitboxes.size(); h1++) {
        
            Hitbox* h1_ptr = &s1_ptr->hitboxes[h1];
            if(h1_ptr->type == HITBOX_TYPE_DISABLED) continue;
            
            for(size_t h2 = 0; h2 < s2_ptr->hitboxes.size(); h2++) {
                Hitbox* h2_ptr = &s2_ptr->hitboxes[h2];
                if(h2_ptr->type == HITBOX_TYPE_DISABLED) continue;
                
                //Get the real hitbox locations.
                Point m1_h_pos =
                    h1_ptr->getCurPos(
                        m_ptr->pos, m_ptr->angleCos, m_ptr->angleSin
                    );
                Point m2_h_pos =
                    h2_ptr->getCurPos(
                        m2_ptr->pos, m2_ptr->angleCos, m2_ptr->angleSin
                    );
                float m1_h_z = m_ptr->z + h1_ptr->z;
                float m2_h_z = m2_ptr->z + h2_ptr->z;
                
                bool collided = false;
                
                if(
                    (
                        m_ptr->holder.m == m2_ptr &&
                        m_ptr->holder.hitboxIdx == h2
                    ) || (
                        m2_ptr->holder.m == m_ptr &&
                        m2_ptr->holder.hitboxIdx == h1
                    )
                ) {
                    //Mobs held by a hitbox are obviously touching it.
                    collided = true;
                }
                
                if(!collided) {
                    bool z_collision;
                    if(h1_ptr->height == 0 || h2_ptr->height == 0) {
                        z_collision = true;
                    } else {
                        z_collision =
                            !(
                                (m2_h_z > m1_h_z + h1_ptr->height) ||
                                (m2_h_z + h2_ptr->height < m1_h_z)
                            );
                    }
                    
                    if(
                        z_collision &&
                        Distance(m1_h_pos, m2_h_pos) <
                        (h1_ptr->radius + h2_ptr->radius)
                    ) {
                        collided = true;
                    }
                }
                
                if(!collided) continue;
                
                //Collision confirmed!
                
                if(
                    hitbox_touch_an_ev && !reported_an_ev &&
                    h1_ptr->type == HITBOX_TYPE_ATTACK &&
                    h2_ptr->type == HITBOX_TYPE_NORMAL
                ) {
                    HitboxInteraction ev_info =
                        HitboxInteraction(
                            m2_ptr, h1_ptr, h2_ptr
                        );
                        
                    hitbox_touch_an_ev->run(
                        m_ptr, (void*) &ev_info
                    );
                    reported_an_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_eat_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_EAT);
                    hitbox_touch_haz_ev =
                        m_ptr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                    hitbox_touch_nn_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_N);
                }
                
                if(
                    hitbox_touch_nn_ev && !reported_nn_ev &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_NORMAL
                ) {
                    HitboxInteraction ev_info =
                        HitboxInteraction(
                            m2_ptr, h1_ptr, h2_ptr
                        );
                        
                    hitbox_touch_nn_ev->run(
                        m_ptr, (void*) &ev_info
                    );
                    reported_nn_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_eat_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_EAT);
                    hitbox_touch_haz_ev =
                        m_ptr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                    hitbox_touch_an_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_A_N);
                }
                
                if(
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_ATTACK
                ) {
                    //Confirmed damage.
                    
                    //Hazard resistance check.
                    if(
                        !h2_ptr->hazards.empty() &&
                        m_ptr->isResistantToHazards(h2_ptr->hazards)
                    ) {
                        continue;
                    }
                    
                    //Should this mob even attack this other mob?
                    if(!m2_ptr->canHurt(m_ptr)) {
                        continue;
                    }
                }
                
                //Check if m2 is under any status effect
                //that disables attacks.
                bool disable_attack_status = false;
                for(size_t s = 0; s < m2_ptr->statuses.size(); s++) {
                    if(m2_ptr->statuses[s].type->disablesAttack) {
                        disable_attack_status = true;
                        break;
                    }
                }
                
                //First, the "touched eat hitbox" event.
                if(
                    hitbox_touch_eat_ev &&
                    !reported_eat_ev &&
                    !disable_attack_status &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    m2_ptr->chompingMobs.size() <
                    m2_ptr->chompMax &&
                    find(
                        m2_ptr->chompBodyParts.begin(),
                        m2_ptr->chompBodyParts.end(),
                        h2_ptr->bodyPartIdx
                    ) !=
                    m2_ptr->chompBodyParts.end()
                ) {
                    hitbox_touch_eat_ev->run(
                        m_ptr,
                        (void*) m2_ptr,
                        (void*) h2_ptr
                    );
                    reported_eat_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_haz_ev =
                        m_ptr->fsm.getEvent(MOB_EV_TOUCHED_HAZARD);
                    hitbox_touch_na_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                }
                
                //"Touched hazard" event.
                if(
                    hitbox_touch_haz_ev &&
                    !reported_haz_ev &&
                    !disable_attack_status &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_ATTACK &&
                    !h2_ptr->hazards.empty()
                ) {
                    for(
                        size_t h = 0;
                        h < h2_ptr->hazards.size(); h++
                    ) {
                        HitboxInteraction ev_info =
                            HitboxInteraction(
                                m2_ptr, h1_ptr, h2_ptr
                            );
                        hitbox_touch_haz_ev->run(
                            m_ptr,
                            (void*) h2_ptr->hazards[h],
                            (void*) &ev_info
                        );
                    }
                    reported_haz_ev = true;
                    
                    //Re-fetch the other events, since this event
                    //could have triggered a state change.
                    hitbox_touch_na_ev =
                        m_ptr->fsm.getEvent(MOB_EV_HITBOX_TOUCH_N_A);
                }
                
                //"Normal hitbox touched attack hitbox" event.
                if(
                    hitbox_touch_na_ev &&
                    !reported_na_ev &&
                    !disable_attack_status &&
                    h1_ptr->type == HITBOX_TYPE_NORMAL &&
                    h2_ptr->type == HITBOX_TYPE_ATTACK
                ) {
                    HitboxInteraction ev_info =
                        HitboxInteraction(
                            m2_ptr, h1_ptr, h2_ptr
                        );
                    hitbox_touch_na_ev->run(
                        m_ptr, (void*) &ev_info
                    );
                    reported_na_ev = true;
                    
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
    markAreaCellsActive(game.cam.box[0], game.cam.box[1]);
}


/**
 * @brief Updates the "isActive" member variable of all mobs for this frame.
 */
void GameplayState::updateMobIsActiveFlag() {
    unordered_set<Mob*> child_mobs;
    
    for(size_t m = 0; m < mobs.all.size(); m++) {
        Mob* m_ptr = mobs.all[m];
        
        int cell_x =
            (m_ptr->pos.x - game.curAreaData->bmap.topLeftCorner.x) /
            GEOMETRY::AREA_CELL_SIZE;
        int cell_y =
            (m_ptr->pos.y - game.curAreaData->bmap.topLeftCorner.y) /
            GEOMETRY::AREA_CELL_SIZE;
        if(
            cell_x < 0 ||
            cell_x >= (int) game.states.gameplay->areaActiveCells.size()
        ) {
            m_ptr->isActive = false;
        } else if(
            cell_y < 0 ||
            cell_y >= (int) game.states.gameplay->areaActiveCells[0].size()
        ) {
            m_ptr->isActive = false;
        } else {
            m_ptr->isActive =
                game.states.gameplay->areaActiveCells[cell_x][cell_y];
        }
        
        if(m_ptr->parent && m_ptr->parent->m) child_mobs.insert(m_ptr);
    }
    
    for(const auto &m : child_mobs) {
        if(m->isActive) m->parent->m->isActive = true;
    }
    
    for(auto &m : child_mobs) {
        if(m->parent->m->isActive) m->isActive = true;
    }
}
