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
 * @brief Does the logic for the dismiss player action.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 */
void GameplayState::doPlayerActionDismiss(Player* player, bool isDown) {
    if(!isDown) return;
    if(!player->leaderPtr) return;
    
    player->leaderPtr->fsm.runEvent(LEADER_EV_DISMISS);
}


/**
 * @brief Does the logic for the lie down player action.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 */
void GameplayState::doPlayerActionLieDown(Player* player, bool isDown) {
    if(!isDown) return;
    if(!player->leaderPtr) return;
    
    player->leaderPtr->fsm.runEvent(LEADER_EV_LIE_DOWN);
}


/**
 * @brief Does the logic for the pause or radar player actions.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 * @param radar Whether to open the radar page.
 */
void GameplayState::doPlayerActionPause(
    Player* player, bool isDown, bool radar
) {
    if(!isDown) return;
    
    pauseMenu = new PauseMenu(radar);
    paused = true;
    game.controls.setGameState(CONTROLS_GAME_STATE_MENUS);
    game.audio.handleWorldPause();
    player->hud->gui.startAnimation(
        GUI_MANAGER_ANIM_IN_TO_OUT,
        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
    player->inventory->close();
}


/**
 * @brief Does the logic for the leader switch player actions.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 * @param isNext Whether it's the action for the next leader or the previous.
 */
void GameplayState::doPlayerActionSwitchLeader(
    Player* player, bool isDown, bool isNext
) {
    if(!isDown) return;
    
    changeToNextLeader(player, isNext, false, false);
}


/**
 * @brief Does the logic for the maturity switch player actions.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 * @param isNext Whether it's the action for the next maturity or the previous.
 */
void GameplayState::doPlayerActionSwitchMaturity(
    Player* player, bool isDown, bool isNext
) {
    if(!isDown) return;
    if(!player->leaderPtr) return;
    
    if(
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
                (int) nextMaturity, isNext ? 1 : -1, N_MATURITIES
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
}


/**
 * @brief Does the logic for the spray switch player actions.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 * @param isNext Whether it's the action for the next spray or the previous.
 */
void GameplayState::doPlayerActionSwitchSpray(
    Player* player, bool isDown, bool isNext
) {
    if(!isDown) return;
    if(!player->leaderPtr) return;
    
    if(game.content.sprayTypes.list.size() > 2) {
        player->selectedSpray =
            sumAndWrap(
                (int) player->selectedSpray,
                isNext ? +1 : -1,
                (int) game.content.sprayTypes.list.size()
            );
        player->hud->spray1Amount->startJuiceAnimation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
        );
    }
}


/**
 * @brief Does the logic for the standby type switch player actions.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 * @param isNext Whether it's the action for the next type or the previous.
 */
void GameplayState::doPlayerActionSwitchType(
    Player* player, bool isDown, bool isNext
) {
    if(!isDown) return;
    if(!player->leaderPtr) return;
    
    if(player->leaderPtr->group->members.empty()) return;
    
    SubgroupType* startingSubgroupType =
        player->leaderPtr->group->curStandbyType;
        
    bool switchSuccessful;
    
    if(player->leaderPtr->holding.empty()) {
        //If the leader isn't holding anybody.
        switchSuccessful =
            player->leaderPtr->group->changeStandbyType(!isNext);
            
    } else {
        //If the leader is holding a Pikmin, we can't let it
        //swap to a Pikmin that's far away.
        //So, every time that happens, skip that subgroup and
        //try the next. Also, make sure to cancel everything if
        //the loop already went through all types.
        
        bool finish = false;
        do {
            switchSuccessful =
                player->leaderPtr->group->changeStandbyType(!isNext);
                
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
        game.audio.createUiSoundSource(
            game.sysContent.sndSwitchPikmin
        );
    }
}


/**
 * @brief Does the logic for the throw player action.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 * @return Whether the action was consumed.
 */
bool GameplayState::doPlayerActionThrow(Player* player, bool isDown) {
    if(!player->leaderPtr) return true;
    
    if(isDown) { //Button press.
        bool done = false;
        
        //Check if the player wants to cancel auto-throw.
        if(
            game.options.controls.autoThrowMode == AUTO_THROW_MODE_TOGGLE &&
            player->leaderPtr->autoThrowRepeater.time != LARGE_FLOAT
        ) {
            player->leaderPtr->stopAutoThrowing();
            done = true;
        }
        
        //Check if the leader should heal themselves on the ship.
        if(
            !done &&
            player->closeToShipToHeal
        ) {
            player->closeToShipToHeal->healLeader(player->leaderPtr);
            done = true;
        }
        
        //Check if the leader should pluck a Pikmin.
        if(
            !done &&
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
            player->inventory->close();
            paused = true;
            game.controls.setGameState(CONTROLS_GAME_STATE_MENUS);
            game.audio.handleWorldPause();
            
            done = true;
        }
        
        //Now check if the leader should interact with an interactable.
        if(
            !done &&
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
        if(!done) {
            MobEvent* ev = player->leaderPtr->fsm.getEvent(LEADER_EV_PUNCH);
            if(ev) {
                ev->run(player->leaderPtr);
                done = true;
            }
        }
        
        if(!done) {
            return false;
        }
        
    } else { //Button release.
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
    
    return true;
}


/**
 * @brief Does the logic for the zoom toggle player action.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 */
void GameplayState::doPlayerActionToggleZoom(Player* player, bool isDown) {
    if(!isDown) return;
    
    if(player->view.cam.targetZoom < zoomLevels[1]) {
        player->view.cam.targetZoom = zoomLevels[0];
    } else if(player->view.cam.targetZoom > zoomLevels[1]) {
        player->view.cam.targetZoom = zoomLevels[1];
    } else {
        if(
            game.options.advanced.zoomMediumReach ==
            game.config.rules.zoomFarthestReach
        ) {
            player->view.cam.targetZoom = zoomLevels[0];
        } else {
            player->view.cam.targetZoom = zoomLevels[2];
        }
    }
    
    game.audio.createUiSoundSource(game.sysContent.sndCamera);
}


/**
 * @brief Does the logic for the current spray usage player action.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 */
void GameplayState::doPlayerActionUseCurrentSpray(Player* player, bool isDown) {
    if(!isDown) return;
    if(!player->leaderPtr) return;
    
    if(game.content.sprayTypes.list.size() > 2) {
        player->leaderPtr->fsm.runEvent(
            LEADER_EV_SPRAY,
            (void*) &player->selectedSpray
        );
    }
}


/**
 * @brief Does the logic for the spray usage player actions.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 * @param second Whether it's the first or the second spray.
 */
void GameplayState::doPlayerActionUseSpray(
    Player* player, bool isDown, bool second
) {
    if(!isDown) return;
    if(!player->leaderPtr) return;
    
    if(!second) {
        if(
            game.content.sprayTypes.list.size() == 1 ||
            game.content.sprayTypes.list.size() == 2
        ) {
            size_t sprayIdx = 0;
            player->leaderPtr->fsm.runEvent(
                LEADER_EV_SPRAY, (void*) &sprayIdx
            );
        }
        
    } else {
        if(game.content.sprayTypes.list.size() == 2) {
            size_t sprayIdx = 1;
            player->leaderPtr->fsm.runEvent(
                LEADER_EV_SPRAY, (void*) &sprayIdx
            );
        }
        
    }
}


/**
 * @brief Does the logic for the whistle player action.
 *
 * @param player The player responsible.
 * @param isDown Whether the input value makes for a "down" or an "up" input.
 */
void GameplayState::doPlayerActionWhistle(Player* player, bool isDown) {
    if(!player->leaderPtr) return;
    
    if(isDown) {
        MobEvent* cancelEv = player->leaderPtr->fsm.getEvent(LEADER_EV_CANCEL);
        
        if(cancelEv) {
            //Cancel auto-pluck, lying down, etc.
            cancelEv->run(player->leaderPtr);
        } else {
            //Start whistling.
            player->leaderPtr->fsm.runEvent(LEADER_EV_START_WHISTLE);
        }
        
    } else {
        //Stop whistling.
        player->leaderPtr->fsm.runEvent(LEADER_EV_STOP_WHISTLE);
        
    }
}


/**
 * @brief Does the logic for the zoom player action.
 *
 * @param player The player responsible.
 * @param inputValue Value of the player input.
 * @param zoomIn Whether it zooms in or out.
 */
void GameplayState::doPlayerActionZoom(
    Player* player, float inputValue, bool zoomIn
) {
    if(player->view.cam.targetZoom >= zoomLevels[0] && zoomIn) {
        return;
    }
    
    if(player->view.cam.targetZoom <= zoomLevels[2] && !zoomIn) {
        return;
    }
    
    float flooredPos = floor(inputValue);
    
    if(zoomIn) {
        player->view.cam.targetZoom =
            player->view.cam.targetZoom + 0.1 * flooredPos;
    } else {
        player->view.cam.targetZoom =
            player->view.cam.targetZoom - 0.1 * flooredPos;
    }
    
    if(player->view.cam.targetZoom > zoomLevels[0]) {
        player->view.cam.targetZoom = zoomLevels[0];
    }
    if(player->view.cam.targetZoom < zoomLevels[2]) {
        player->view.cam.targetZoom = zoomLevels[2];
    }
    
    game.audio.createUiSoundSource(
        game.sysContent.sndCamera,
    { .stackMode = SOUND_STACK_MODE_NEVER }
    );
}


/**
 * @brief Handles a player action.
 *
 * @param action Data about the action.
 */
void GameplayState::handlePlayerAction(const Inpution::Action& action) {
    if(shouldIgnorePlayerAction(action)) return;
    
    Player* player = &players[0];
    bool isDown = (action.value >= 0.5);
    
    //Before we do the actions, we'll tell the leader object
    //it's received an input, which will trigger an event.
    if(player->leaderPtr) {
        player->leaderPtr->fsm.runEvent(
            MOB_EV_INPUT_RECEIVED,
            (void*) &action
        );
    }
    
    if(!msgBox && !onionMenu && !pauseMenu && !player->inventory->isOpen) {
    
        switch(action.actionTypeId) {
        case PLAYER_ACTION_TYPE_RIGHT:
        case PLAYER_ACTION_TYPE_DOWN:
        case PLAYER_ACTION_TYPE_LEFT:
        case PLAYER_ACTION_TYPE_UP: {
            /*******************
            *               O_ *
            *   Move   --->/|  *
            *              V > *
            *******************/
            
            switch(action.actionTypeId) {
            case PLAYER_ACTION_TYPE_RIGHT: {
                player->leaderMovement.right = action.value;
                break;
            } case PLAYER_ACTION_TYPE_DOWN: {
                player->leaderMovement.down = action.value;
                break;
            } case PLAYER_ACTION_TYPE_LEFT: {
                player->leaderMovement.left = action.value;
                break;
            } case PLAYER_ACTION_TYPE_UP: {
                player->leaderMovement.up = action.value;
                break;
            } default: {
                break;
            }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_LEADER_CURSOR_RIGHT:
        case PLAYER_ACTION_TYPE_LEADER_CURSOR_DOWN:
        case PLAYER_ACTION_TYPE_LEADER_CURSOR_LEFT:
        case PLAYER_ACTION_TYPE_LEADER_CURSOR_UP: {
            /***************************
            *                    .-.   *
            *   Leader cursor   ( = )> *
            *                    '-'   *
            ***************************/
            
            switch(action.actionTypeId) {
            case PLAYER_ACTION_TYPE_LEADER_CURSOR_RIGHT: {
                player->leaderCursorMov.right = action.value;
                break;
            } case PLAYER_ACTION_TYPE_LEADER_CURSOR_DOWN: {
                player->leaderCursorMov.down = action.value;
                break;
            } case PLAYER_ACTION_TYPE_LEADER_CURSOR_LEFT: {
                player->leaderCursorMov.left = action.value;
                break;
            } case PLAYER_ACTION_TYPE_LEADER_CURSOR_UP: {
                player->leaderCursorMov.up = action.value;
                break;
            } default: {
                break;
            }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_GROUP_RIGHT:
        case PLAYER_ACTION_TYPE_GROUP_DOWN:
        case PLAYER_ACTION_TYPE_GROUP_LEFT:
        case PLAYER_ACTION_TYPE_GROUP_UP: {
            /******************
            *            ***  *
            *   Group   ****O *
            *            ***  *
            ******************/
            
            switch(action.actionTypeId) {
            case PLAYER_ACTION_TYPE_GROUP_RIGHT: {
                player->swarmMovement.right = action.value;
                break;
            } case PLAYER_ACTION_TYPE_GROUP_DOWN: {
                player->swarmMovement.down = action.value;
                break;
            } case PLAYER_ACTION_TYPE_GROUP_LEFT: {
                player->swarmMovement.left = action.value;
                break;
            } case PLAYER_ACTION_TYPE_GROUP_UP: {
                player->swarmMovement.up = action.value;
                break;
            } default: {
                break;
            }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_GROUP_CURSOR: {
    
            player->swarmToLeaderCursor = isDown;
            
            break;
            
        } case PLAYER_ACTION_TYPE_THROW: {
            if(!doPlayerActionThrow(player, isDown)) {
                game.controls.reinsertAction(action);
            }
            break;
            
        } case PLAYER_ACTION_TYPE_WHISTLE: {
            doPlayerActionWhistle(player, isDown);
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_LEADER:
        case PLAYER_ACTION_TYPE_PREV_LEADER: {
            doPlayerActionSwitchLeader(
                player, isDown,
                action.actionTypeId == PLAYER_ACTION_TYPE_NEXT_LEADER
            );
            break;
            
        } case PLAYER_ACTION_TYPE_DISMISS: {
            doPlayerActionDismiss(player, isDown);
            break;
            
        } case PLAYER_ACTION_TYPE_PAUSE:
        case PLAYER_ACTION_TYPE_RADAR: {
            doPlayerActionPause(
                player, isDown,
                action.actionTypeId == PLAYER_ACTION_TYPE_RADAR
            );
            break;
            
        } case PLAYER_ACTION_TYPE_INVENTORY: {
            if(isDown) player->inventory->open();
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY_1:
        case PLAYER_ACTION_TYPE_USE_SPRAY_2: {
            doPlayerActionUseSpray(
                player, isDown,
                action.actionTypeId == PLAYER_ACTION_TYPE_USE_SPRAY_2
            );
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_SPRAY:
        case PLAYER_ACTION_TYPE_PREV_SPRAY: {
            doPlayerActionSwitchSpray(
                player, isDown,
                action.actionTypeId == PLAYER_ACTION_TYPE_NEXT_SPRAY
            );
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY: {
            doPlayerActionUseCurrentSpray(player, isDown);
            break;
            
        } case PLAYER_ACTION_TYPE_CHANGE_ZOOM: {
            doPlayerActionToggleZoom(player, isDown);
            break;
            
        } case PLAYER_ACTION_TYPE_ZOOM_IN:
        case PLAYER_ACTION_TYPE_ZOOM_OUT: {
            doPlayerActionZoom(
                player, action.value,
                action.actionTypeId == PLAYER_ACTION_TYPE_ZOOM_IN
            );
            break;
            
        } case PLAYER_ACTION_TYPE_LIE_DOWN: {
            doPlayerActionLieDown(player, isDown);
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_TYPE:
        case PLAYER_ACTION_TYPE_PREV_TYPE: {
            doPlayerActionSwitchType(
                player, isDown,
                action.actionTypeId == PLAYER_ACTION_TYPE_NEXT_TYPE
            );
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_MATURITY:
        case PLAYER_ACTION_TYPE_PREV_MATURITY: {
            doPlayerActionSwitchMaturity(
                player, isDown,
                action.actionTypeId == PLAYER_ACTION_TYPE_NEXT_MATURITY
            );
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
        
    } else if(player->inventory->isOpen) {
        //In the inventory.
        player->inventory->handlePlayerAction(action);
        
    }
    
}


/**
 * @brief Returns whether a given player action should be ignored, based
 * on the state of the game.
 *
 * @param action Action to check.
 * @return Whether it should be ignored.
 */
bool GameplayState::shouldIgnorePlayerAction(const Inpution::Action& action) {
    const vector<int> actionsAllowedDuringInterludes {
        PLAYER_ACTION_TYPE_CHANGE_ZOOM,
        PLAYER_ACTION_TYPE_LEADER_CURSOR_RIGHT,
        PLAYER_ACTION_TYPE_LEADER_CURSOR_DOWN,
        PLAYER_ACTION_TYPE_LEADER_CURSOR_LEFT,
        PLAYER_ACTION_TYPE_LEADER_CURSOR_UP,
        PLAYER_ACTION_TYPE_ZOOM_IN,
        PLAYER_ACTION_TYPE_ZOOM_OUT,
    };
    
    if(!readyForInput || !isInputAllowed) return true;
    if(interlude.get() != INTERLUDE_NONE) {
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
