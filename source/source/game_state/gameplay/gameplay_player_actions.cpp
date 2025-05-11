/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Control handling in the gameplay state.
 */

#include <algorithm>

#include "gameplay.h"

#include "../../core/game.h"


/**
 * @brief Handles a player action.
 *
 * @param action Data about the action.
 */
void GameplayState::handlePlayerAction(const PlayerAction &action) {
    if(shouldIngorePlayerAction(action)) return;
    
    Player* player = &players[0];
    bool isDown = (action.value >= 0.5);
    
    //Before we do the actions, we'll tell the leader object
    //it's recieved an input, which will trigger an event.
    if(player->leaderPtr) {
        player->leaderPtr->fsm.runEvent(
            MOB_EV_INPUT_RECEIVED,
            (void*) &action
        );
    }
    
    if(!msgBox && !onionMenu && !pauseMenu) {
    
        switch(action.actionTypeId) {
        case PLAYER_ACTION_TYPE_THROW: {
    
            /*******************
            *             .-.  *
            *   Throw    /   O *
            *           &      *
            *******************/
            
            if(isDown) { //Button press.
            
                bool done = false;
                
                //Check if the player wants to cancel auto-throw.
                if(
                    player->leaderPtr &&
                    game.options.controls.autoThrowMode == AUTO_THROW_MODE_TOGGLE &&
                    player->leaderPtr->autoThrowRepeater.time != LARGE_FLOAT
                ) {
                    player->leaderPtr->stopAutoThrowing();
                    done = true;
                }
                
                //Check if the leader should heal themselves on the ship.
                if(
                    !done &&
                    player->leaderPtr &&
                    player->closeToShipToHeal
                ) {
                    player->closeToShipToHeal->healLeader(player->leaderPtr);
                    done = true;
                }
                
                //Check if the leader should pluck a Pikmin.
                if(
                    !done &&
                    player->leaderPtr &&
                    player->closeToPikminToPluck
                ) {
                    player->leaderPtr->fsm.runEvent(
                        LEADER_EV_GO_PLUCK,
                        (void*) player->closeToPikminToPluck
                    );
                    done = true;
                }
                
                //Now check if the leader should open an Onion's menu.
                if(
                    !done &&
                    player->leaderPtr &&
                    player->closeToNestToOpen
                ) {
                    onionMenu = new OnionMenu(
                        player->closeToNestToOpen,
                        player->leaderPtr
                    );
                    player->hud->gui.startAnimation(
                        GUI_MANAGER_ANIM_IN_TO_OUT,
                        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
                    );
                    paused = true;
                    game.audio.handleWorldPause();
                    
                    //TODO replace with a better solution.
                    player->leaderPtr->fsm.runEvent(LEADER_EV_STOP_WHISTLE);
                    
                    done = true;
                }
                
                //Now check if the leader should interact with an interactable.
                if(
                    !done &&
                    player->leaderPtr &&
                    player->closeToInteractableToUse
                ) {
                    string msg = "interact";
                    player->leaderPtr->sendScriptMessage(
                        player->closeToInteractableToUse, msg
                    );
                    done = true;
                }
                
                //Now check if the leader should grab a Pikmin.
                if(
                    !done &&
                    player->leaderPtr &&
                    player->leaderPtr->holding.empty() &&
                    player->leaderPtr->group->curStandbyType &&
                    !player->closestGroupMemberDistant
                ) {
                    switch (game.options.controls.autoThrowMode) {
                    case AUTO_THROW_MODE_OFF: {
                        done = grabClosestGroupMember(player);
                        break;
                    } case AUTO_THROW_MODE_HOLD:
                    case AUTO_THROW_MODE_TOGGLE: {
                        player->leaderPtr->startAutoThrowing();
                        done = true;
                        break;
                    }
                    default: {
                        break;
                    }
                    }
                }
                
                //Now check if the leader should punch.
                if(
                    !done &&
                    player->leaderPtr
                ) {
                    player->leaderPtr->fsm.runEvent(LEADER_EV_PUNCH);
                    done = true;
                }
                
            } else { //Button release.
            
                if(player->leaderPtr) {
                    switch (game.options.controls.autoThrowMode) {
                    case AUTO_THROW_MODE_OFF: {
                        player->leaderPtr->queueThrow();
                        break;
                    } case AUTO_THROW_MODE_HOLD: {
                        player->leaderPtr->stopAutoThrowing();
                        break;
                    } case AUTO_THROW_MODE_TOGGLE: {
                        break;
                    } default: {
                        break;
                    }
                    }
                }
                
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_WHISTLE: {
    
            /********************
            *              .--= *
            *   Whistle   ( @ ) *
            *              '-'  *
            ********************/
            
            if(isDown) {
                //Button pressed.
                
                if(player->leaderPtr) {
                    MobEvent* cancelEv =
                        player->leaderPtr->fsm.getEvent(LEADER_EV_CANCEL);
                        
                    if(cancelEv) {
                        //Cancel auto-pluck, lying down, etc.
                        cancelEv->run(player->leaderPtr);
                    } else {
                        //Start whistling.
                        player->leaderPtr->fsm.runEvent(LEADER_EV_START_WHISTLE);
                    }
                }
                
            } else {
                //Button released.
                
                if(player->leaderPtr) {
                    player->leaderPtr->fsm.runEvent(LEADER_EV_STOP_WHISTLE);
                }
                
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_LEADER:
        case PLAYER_ACTION_TYPE_PREV_LEADER: {
    
            /******************************
            *                    \O/  \O/ *
            *   Switch leader     | -> |  *
            *                    / \  / \ *
            ******************************/
            
            if(!isDown) return;
            
            changeToNextLeader(
                player,
                action.actionTypeId == PLAYER_ACTION_TYPE_NEXT_LEADER,
                false, false
            );
            
            break;
            
        } case PLAYER_ACTION_TYPE_DISMISS: {
    
            /***********************
            *             \O/ / *  *
            *   Dismiss    |   - * *
            *             / \ \ *  *
            ***********************/
            
            if(!isDown) return;
            
            if(player->leaderPtr) {
                player->leaderPtr->fsm.runEvent(LEADER_EV_DISMISS);
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_PAUSE:
        case PLAYER_ACTION_TYPE_RADAR: {
    
            /********************
            *           +-+ +-+ *
            *   Pause   | | | | *
            *           +-+ +-+ *
            ********************/
            
            if(!isDown) return;
            
            pauseMenu =
                new PauseMenu(
                action.actionTypeId == PLAYER_ACTION_TYPE_RADAR
            );
            paused = true;
            game.audio.handleWorldPause();
            player->hud->gui.startAnimation(
                GUI_MANAGER_ANIM_IN_TO_OUT,
                GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
            );
            
            //TODO replace with a better solution.
            if(player->leaderPtr) {
                player->leaderPtr->fsm.runEvent(LEADER_EV_STOP_WHISTLE);
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY_1: {
    
            /*******************
            *             +=== *
            *   Sprays   (   ) *
            *             '-'  *
            *******************/
            
            if(!isDown) return;
            
            if(player->leaderPtr) {
                if(
                    game.content.sprayTypes.list.size() == 1 ||
                    game.content.sprayTypes.list.size() == 2
                ) {
                    size_t sprayIdx = 0;
                    player->leaderPtr->fsm.runEvent(
                        LEADER_EV_SPRAY, (void*) &sprayIdx
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY_2: {
    
            if(!isDown) return;
            
            if(player->leaderPtr) {
                if(game.content.sprayTypes.list.size() == 2) {
                    size_t sprayIdx = 1;
                    player->leaderPtr->fsm.runEvent(
                        LEADER_EV_SPRAY, (void*) &sprayIdx
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_SPRAY:
        case PLAYER_ACTION_TYPE_PREV_SPRAY: {
    
            if(!isDown) return;
            
            if(player->leaderPtr) {
                if(game.content.sprayTypes.list.size() > 2) {
                    player->selectedSpray =
                        sumAndWrap(
                            (int) player->selectedSpray,
                            action.actionTypeId ==
                            PLAYER_ACTION_TYPE_NEXT_SPRAY ? +1 : -1,
                            (int) game.content.sprayTypes.list.size()
                        );
                    player->hud->
                    spray1Amount->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY: {
    
            if(!isDown) return;
            
            if(player->leaderPtr) {
                if(game.content.sprayTypes.list.size() > 2) {
                    player->leaderPtr->fsm.runEvent(
                        LEADER_EV_SPRAY,
                        (void*) &player->selectedSpray
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_CHANGE_ZOOM: {
    
            /***************
            *           _  *
            *   Zoom   (_) *
            *          /   *
            ***************/
            
            if(!isDown) return;
            
            if(player->view.cam.targetZoom < zoomLevels[1]) {
                player->view.cam.targetZoom = zoomLevels[0];
            } else if(player->view.cam.targetZoom > zoomLevels[1]) {
                player->view.cam.targetZoom = zoomLevels[1];
            } else {
                if(game.options.advanced.zoomMediumReach == game.config.rules.zoomFarthestReach) {
                    player->view.cam.targetZoom = zoomLevels[0];
                } else {
                    player->view.cam.targetZoom = zoomLevels[2];
                }
            }
            
            game.audio.createUiSoundsource(game.sysContent.sndCamera);
            
            break;
            
        } case PLAYER_ACTION_TYPE_ZOOM_IN:
        case PLAYER_ACTION_TYPE_ZOOM_OUT: {
    
            if(
                player->view.cam.targetZoom >= zoomLevels[0] &&
                action.actionTypeId == PLAYER_ACTION_TYPE_ZOOM_IN
            ) {
                return;
            }
            
            if(
                player->view.cam.targetZoom <= zoomLevels[2] &&
                action.actionTypeId == PLAYER_ACTION_TYPE_ZOOM_OUT
            ) {
                return;
            }
            
            float flooredPos = floor(action.value);
            
            if(action.actionTypeId == PLAYER_ACTION_TYPE_ZOOM_IN) {
                player->view.cam.targetZoom = player->view.cam.targetZoom + 0.1 * flooredPos;
            } else {
                player->view.cam.targetZoom = player->view.cam.targetZoom - 0.1 * flooredPos;
            }
            
            if(player->view.cam.targetZoom > zoomLevels[0]) {
                player->view.cam.targetZoom = zoomLevels[0];
            }
            if(player->view.cam.targetZoom < zoomLevels[2]) {
                player->view.cam.targetZoom = zoomLevels[2];
            }
            
            SoundSourceConfig camSoundConfig;
            camSoundConfig.stackMode = SOUND_STACK_MODE_NEVER;
            game.audio.createUiSoundsource(
                game.sysContent.sndCamera,
                camSoundConfig
            );
            
            break;
            
        } case PLAYER_ACTION_TYPE_LIE_DOWN: {
    
            /**********************
            *                     *
            *   Lie down  -()/__/ *
            *                     *
            **********************/
            
            if(!isDown) return;
            
            if(player->leaderPtr) {
                player->leaderPtr->fsm.runEvent(LEADER_EV_LIE_DOWN);
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_TYPE:
        case PLAYER_ACTION_TYPE_PREV_TYPE: {
    
            /****************************
            *                     -->   *
            *   Switch type   <( )> (o) *
            *                           *
            ****************************/
            
            if(!isDown) return;
            
            if(player->leaderPtr) {
                if(player->leaderPtr->group->members.empty()) return;
                
                SubgroupType* startingSubgroupType =
                    player->leaderPtr->group->curStandbyType;
                    
                bool switchSuccessful;
                
                if(player->leaderPtr->holding.empty()) {
                    //If the leader isn't holding anybody.
                    switchSuccessful =
                        player->leaderPtr->group->changeStandbyType(
                            action.actionTypeId == PLAYER_ACTION_TYPE_PREV_TYPE
                        );
                        
                } else {
                    //If the leader is holding a Pikmin, we can't let it
                    //swap to a Pikmin that's far away.
                    //So, every time that happens, skip that subgroup and
                    //try the next. Also, make sure to cancel everything if
                    //the loop already went through all types.
                    
                    bool finish = false;
                    do {
                        switchSuccessful =
                            player->leaderPtr->group->changeStandbyType(
                                action.actionTypeId == PLAYER_ACTION_TYPE_PREV_TYPE
                            );
                            
                        if(
                            !switchSuccessful ||
                            player->leaderPtr->group->curStandbyType ==
                            startingSubgroupType
                        ) {
                            //Reached around back to the first subgroup...
                            switchSuccessful = false;
                            finish = true;
                            
                        } else {
                            //Switched to a new subgroup.
                            updateClosestGroupMembers(player);
                            if(!player->closestGroupMemberDistant) {
                                finish = true;
                            }
                            
                        }
                        
                    } while(!finish);
                    
                    if(switchSuccessful) {
                        player->leaderPtr->swapHeldPikmin(
                            player->closestGroupMember[BUBBLE_RELATION_CURRENT]
                        );
                    }
                }
                
                if(switchSuccessful) {
                    game.audio.createUiSoundsource(
                        game.sysContent.sndSwitchPikmin
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_MATURITY:
        case PLAYER_ACTION_TYPE_PREV_MATURITY: {
    
            /**********************************
            *                      V  -->  *  *
            *   Switch maturity    |       |  *
            *                     ( )     ( ) *
            **********************************/
            
            if(
                !isDown ||
                !player->leaderPtr ||
                player->leaderPtr->holding.empty() ||
                player->leaderPtr->holding[0]->type->category->id !=
                MOB_CATEGORY_PIKMIN
            ) {
                return;
            }
            
            Pikmin* heldPPtr = (Pikmin*) player->leaderPtr->holding[0];
            
            Pikmin* closestMembers[N_MATURITIES];
            Distance closestDists[N_MATURITIES];
            for(size_t m = 0; m < N_MATURITIES; m++) {
                closestMembers[m] = nullptr;
            }
            
            for(size_t m = 0; m < player->leaderPtr->group->members.size(); m++) {
                Mob* mPtr = player->leaderPtr->group->members[m];
                if(mPtr->type != heldPPtr->type) continue;
                
                Pikmin* pPtr = (Pikmin*) mPtr;
                if(pPtr->maturity == heldPPtr->maturity) continue;
                
                Distance d(player->leaderPtr->pos, pPtr->pos);
                if(
                    !closestMembers[pPtr->maturity] ||
                    d < closestDists[pPtr->maturity]
                ) {
                    closestMembers[pPtr->maturity] = pPtr;
                    closestDists[pPtr->maturity] = d;
                }
                
            }
            
            size_t nextMaturity = heldPPtr->maturity;
            Mob* newPikmin = nullptr;
            bool finished = false;
            do {
                nextMaturity =
                    (size_t) sumAndWrap(
                        (int) nextMaturity,
                        (
                            action.actionTypeId ==
                            PLAYER_ACTION_TYPE_NEXT_MATURITY ? 1 : -1
                        ),
                        N_MATURITIES
                    );
                    
                //Back to the start?
                if(nextMaturity == heldPPtr->maturity) break;
                
                if(!closestMembers[nextMaturity]) continue;
                
                newPikmin = closestMembers[nextMaturity];
                finished = true;
                
            } while(!finished);
            
            if(newPikmin) {
                player->leaderPtr->swapHeldPikmin(newPikmin);
            }
            
            break;
            
        }
        default: {
            break;
        }
        }
        
    } else if(msgBox) {
    
        //Displaying a message.
        if(action.actionTypeId == PLAYER_ACTION_TYPE_THROW && isDown) {
            msgBox->advance();
        } else if(action.actionTypeId == PLAYER_ACTION_TYPE_PAUSE && isDown) {
            msgBox->close();
        }
        
    }
    //Some inputs we don't want to ignore even if we're in a menu.
    //Those go here.
    switch (action.actionTypeId) {
    case PLAYER_ACTION_TYPE_RIGHT:
    case PLAYER_ACTION_TYPE_UP:
    case PLAYER_ACTION_TYPE_LEFT:
    case PLAYER_ACTION_TYPE_DOWN: {
        /*******************
        *               O_ *
        *   Move   --->/|  *
        *              V > *
        *******************/
        
        switch(action.actionTypeId) {
        case PLAYER_ACTION_TYPE_RIGHT: {
            player->leaderMovement.right = action.value;
            break;
        } case PLAYER_ACTION_TYPE_LEFT: {
            player->leaderMovement.left = action.value;
            break;
        } case PLAYER_ACTION_TYPE_UP: {
            player->leaderMovement.up = action.value;
            break;
        } case PLAYER_ACTION_TYPE_DOWN: {
            player->leaderMovement.down = action.value;
            break;
        } default: {
            break;
        }
        }
        
        break;
        
    } case PLAYER_ACTION_TYPE_CURSOR_RIGHT:
    case PLAYER_ACTION_TYPE_CURSOR_UP:
    case PLAYER_ACTION_TYPE_CURSOR_LEFT:
    case PLAYER_ACTION_TYPE_CURSOR_DOWN: {
        /********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             '-'   *
        ********************/
        
        switch(action.actionTypeId) {
        case PLAYER_ACTION_TYPE_CURSOR_RIGHT: {
            player->cursorMovement.right = action.value;
            break;
        } case PLAYER_ACTION_TYPE_CURSOR_LEFT: {
            player->cursorMovement.left = action.value;
            break;
        } case PLAYER_ACTION_TYPE_CURSOR_UP: {
            player->cursorMovement.up = action.value;
            break;
        } case PLAYER_ACTION_TYPE_CURSOR_DOWN: {
            player->cursorMovement.down = action.value;
            break;
        } default: {
            break;
        }
        }
        
        break;
        
    } case PLAYER_ACTION_TYPE_GROUP_RIGHT:
    case PLAYER_ACTION_TYPE_GROUP_UP:
    case PLAYER_ACTION_TYPE_GROUP_LEFT:
    case PLAYER_ACTION_TYPE_GROUP_DOWN: {
        /******************
        *            ***  *
        *   Group   ****O *
        *            ***  *
        ******************/
        
        switch(action.actionTypeId) {
        case PLAYER_ACTION_TYPE_GROUP_RIGHT: {
            player->swarmMovement.right = action.value;
            break;
        } case PLAYER_ACTION_TYPE_GROUP_LEFT: {
            player->swarmMovement.left = action.value;
            break;
        } case PLAYER_ACTION_TYPE_GROUP_UP: {
            player->swarmMovement.up = action.value;
            break;
        } case PLAYER_ACTION_TYPE_GROUP_DOWN: {
            player->swarmMovement.down = action.value;
            break;
        } default: {
            break;
        }
        }
        
        break;
        
    } case PLAYER_ACTION_TYPE_GROUP_CURSOR: {

        player->swarmCursor = isDown;
        
        break;
        
    } default: {
        break;
    }
    }
    
}


/**
 * @brief Returns whether a given player action should be ignored, based
 * on the state of the game.
 *
 * @param action Action to check.
 * @return Whether it should be ignored.
 */
bool GameplayState::shouldIngorePlayerAction(const PlayerAction &action) {
    const vector<int> actionsAllowedDuringInterludes {
        PLAYER_ACTION_TYPE_CHANGE_ZOOM,
        PLAYER_ACTION_TYPE_CURSOR_DOWN,
        PLAYER_ACTION_TYPE_CURSOR_LEFT,
        PLAYER_ACTION_TYPE_CURSOR_RIGHT,
        PLAYER_ACTION_TYPE_CURSOR_UP,
        PLAYER_ACTION_TYPE_ZOOM_IN,
        PLAYER_ACTION_TYPE_ZOOM_OUT,
    };
    
    if(!readyForInput || !isInputAllowed) return true;
    if(curInterlude != INTERLUDE_NONE) {
        if(
            !isInContainer(
                actionsAllowedDuringInterludes, action.actionTypeId
            )
        ) {
            return true;
        }
    }
    
    return false;
}
