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
    
    bool is_down = (action.value >= 0.5);
    
    //Before we do the actions, we'll tell the leader object
    //it's recieved an input, which will trigger an event.
    if(curLeaderPtr) {
        curLeaderPtr->fsm.runEvent(
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
            
            if(is_down) { //Button press.
            
                bool done = false;
                
                //Check if the player wants to cancel auto-throw.
                if(
                    curLeaderPtr &&
                    game.options.controls.autoThrowMode == AUTO_THROW_MODE_TOGGLE &&
                    curLeaderPtr->autoThrowRepeater.time != LARGE_FLOAT
                ) {
                    curLeaderPtr->stopAutoThrowing();
                    done = true;
                }
                
                //Check if the leader should heal themselves on the ship.
                if(
                    !done &&
                    curLeaderPtr &&
                    closeToShipToHeal
                ) {
                    closeToShipToHeal->healLeader(curLeaderPtr);
                    done = true;
                }
                
                //Check if the leader should pluck a Pikmin.
                if(
                    !done &&
                    curLeaderPtr &&
                    closeToPikminToPluck
                ) {
                    curLeaderPtr->fsm.runEvent(
                        LEADER_EV_GO_PLUCK,
                        (void*)closeToPikminToPluck
                    );
                    done = true;
                }
                
                //Now check if the leader should open an Onion's menu.
                if(
                    !done &&
                    curLeaderPtr &&
                    closeToNestToOpen
                ) {
                    onionMenu = new OnionMenu(
                        closeToNestToOpen,
                        curLeaderPtr
                    );
                    hud->gui.startAnimation(
                        GUI_MANAGER_ANIM_IN_TO_OUT,
                        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
                    );
                    paused = true;
                    game.audio.handleWorldPause();
                    
                    //TODO replace with a better solution.
                    curLeaderPtr->fsm.runEvent(LEADER_EV_STOP_WHISTLE);
                    
                    done = true;
                }
                
                //Now check if the leader should interact with an interactable.
                if(
                    !done &&
                    curLeaderPtr &&
                    closeToInteractableToUse
                ) {
                    string msg = "interact";
                    curLeaderPtr->sendScriptMessage(
                        closeToInteractableToUse, msg
                    );
                    done = true;
                }
                
                //Now check if the leader should grab a Pikmin.
                if(
                    !done &&
                    curLeaderPtr &&
                    curLeaderPtr->holding.empty() &&
                    curLeaderPtr->group->curStandbyType &&
                    !closestGroupMemberDistant
                ) {
                    switch (game.options.controls.autoThrowMode) {
                    case AUTO_THROW_MODE_OFF: {
                        done = grabClosestGroupMember();
                        break;
                    } case AUTO_THROW_MODE_HOLD:
                    case AUTO_THROW_MODE_TOGGLE: {
                        curLeaderPtr->startAutoThrowing();
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
                    curLeaderPtr
                ) {
                    curLeaderPtr->fsm.runEvent(LEADER_EV_PUNCH);
                    done = true;
                }
                
            } else { //Button release.
            
                if(curLeaderPtr) {
                    switch (game.options.controls.autoThrowMode) {
                    case AUTO_THROW_MODE_OFF: {
                        curLeaderPtr->queueThrow();
                        break;
                    } case AUTO_THROW_MODE_HOLD: {
                        curLeaderPtr->stopAutoThrowing();
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
            
            if(is_down) {
                //Button pressed.
                
                if(curLeaderPtr) {
                    MobEvent* cancel_ev =
                        curLeaderPtr->fsm.getEvent(LEADER_EV_CANCEL);
                        
                    if(cancel_ev) {
                        //Cancel auto-pluck, lying down, etc.
                        cancel_ev->run(curLeaderPtr);
                    } else {
                        //Start whistling.
                        curLeaderPtr->fsm.runEvent(LEADER_EV_START_WHISTLE);
                    }
                }
                
            } else {
                //Button released.
                
                if(curLeaderPtr) {
                    curLeaderPtr->fsm.runEvent(LEADER_EV_STOP_WHISTLE);
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
            
            if(!is_down) return;
            
            changeToNextLeader(
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
            
            if(!is_down) return;
            
            if(curLeaderPtr && !curLeaderPtr->group->members.empty()) {
                curLeaderPtr->fsm.runEvent(LEADER_EV_DISMISS);
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_PAUSE:
        case PLAYER_ACTION_TYPE_RADAR: {
    
            /********************
            *           +-+ +-+ *
            *   Pause   | | | | *
            *           +-+ +-+ *
            ********************/
            
            if(!is_down) return;
            
            pauseMenu =
                new PauseMenu(
                action.actionTypeId == PLAYER_ACTION_TYPE_RADAR
            );
            paused = true;
            game.audio.handleWorldPause();
            hud->gui.startAnimation(
                GUI_MANAGER_ANIM_IN_TO_OUT,
                GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
            );
            
            //TODO replace with a better solution.
            if(curLeaderPtr) {
                curLeaderPtr->fsm.runEvent(LEADER_EV_STOP_WHISTLE);
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY_1: {
    
            /*******************
            *             +=== *
            *   Sprays   (   ) *
            *             '-'  *
            *******************/
            
            if(!is_down) return;
            
            if(curLeaderPtr) {
                if(
                    game.content.sprayTypes.list.size() == 1 ||
                    game.content.sprayTypes.list.size() == 2
                ) {
                    size_t spray_idx = 0;
                    curLeaderPtr->fsm.runEvent(
                        LEADER_EV_SPRAY, (void*) &spray_idx
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY_2: {
    
            if(!is_down) return;
            
            if(curLeaderPtr) {
                if(game.content.sprayTypes.list.size() == 2) {
                    size_t spray_idx = 1;
                    curLeaderPtr->fsm.runEvent(
                        LEADER_EV_SPRAY, (void*) &spray_idx
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_SPRAY:
        case PLAYER_ACTION_TYPE_PREV_SPRAY: {
    
            if(!is_down) return;
            
            if(curLeaderPtr) {
                if(game.content.sprayTypes.list.size() > 2) {
                    selectedSpray =
                        sumAndWrap(
                            (int) selectedSpray,
                            action.actionTypeId ==
                            PLAYER_ACTION_TYPE_NEXT_SPRAY ? +1 : -1,
                            (int) game.content.sprayTypes.list.size()
                        );
                    game.states.gameplay->hud->
                    spray1Amount->startJuiceAnimation(
                        GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_USE_SPRAY: {
    
            if(!is_down) return;
            
            if(curLeaderPtr) {
                if(game.content.sprayTypes.list.size() > 2) {
                    curLeaderPtr->fsm.runEvent(
                        LEADER_EV_SPRAY,
                        (void*) &selectedSpray
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
            
            if(!is_down) return;
            
            if(game.view.cam.targetZoom < game.options.advanced.zoomMidLevel) {
                game.view.cam.targetZoom = game.config.rules.zoomMaxLevel;
            } else if(game.view.cam.targetZoom > game.options.advanced.zoomMidLevel) {
                game.view.cam.targetZoom = game.options.advanced.zoomMidLevel;
            } else {
                if(game.options.advanced.zoomMidLevel == game.config.rules.zoomMinLevel) {
                    game.view.cam.targetZoom = game.config.rules.zoomMaxLevel;
                } else {
                    game.view.cam.targetZoom = game.config.rules.zoomMinLevel;
                }
            }
            
            game.audio.createUiSoundsource(game.sysContent.sndCamera);
            
            break;
            
        } case PLAYER_ACTION_TYPE_ZOOM_IN:
        case PLAYER_ACTION_TYPE_ZOOM_OUT: {
    
            if(
                game.view.cam.targetZoom >= game.config.rules.zoomMaxLevel &&
                action.actionTypeId == PLAYER_ACTION_TYPE_ZOOM_IN
            ) {
                return;
            }
            
            if(
                game.view.cam.targetZoom <= game.config.rules.zoomMinLevel &&
                action.actionTypeId == PLAYER_ACTION_TYPE_ZOOM_OUT
            ) {
                return;
            }
            
            float floored_pos = floor(action.value);
            
            if(action.actionTypeId == PLAYER_ACTION_TYPE_ZOOM_IN) {
                game.view.cam.targetZoom = game.view.cam.targetZoom + 0.1 * floored_pos;
            } else {
                game.view.cam.targetZoom = game.view.cam.targetZoom - 0.1 * floored_pos;
            }
            
            if(game.view.cam.targetZoom > game.config.rules.zoomMaxLevel) {
                game.view.cam.targetZoom = game.config.rules.zoomMaxLevel;
            }
            if(game.view.cam.targetZoom < game.config.rules.zoomMinLevel) {
                game.view.cam.targetZoom = game.config.rules.zoomMinLevel;
            }
            
            SoundSourceConfig cam_sound_config;
            cam_sound_config.stackMode = SOUND_STACK_MODE_NEVER;
            game.audio.createUiSoundsource(
                game.sysContent.sndCamera,
                cam_sound_config
            );
            
            break;
            
        } case PLAYER_ACTION_TYPE_LIE_DOWN: {
    
            /**********************
            *                     *
            *   Lie down  -()/__/ *
            *                     *
            **********************/
            
            if(!is_down) return;
            
            if(curLeaderPtr) {
                curLeaderPtr->fsm.runEvent(LEADER_EV_LIE_DOWN);
            }
            
            break;
            
        } case PLAYER_ACTION_TYPE_NEXT_TYPE:
        case PLAYER_ACTION_TYPE_PREV_TYPE: {
    
            /****************************
            *                     -->   *
            *   Switch type   <( )> (o) *
            *                           *
            ****************************/
            
            if(!is_down) return;
            
            if(curLeaderPtr) {
                if(curLeaderPtr->group->members.empty()) return;
                
                SubgroupType* starting_subgroup_type =
                    curLeaderPtr->group->curStandbyType;
                    
                bool switch_successful;
                
                if(curLeaderPtr->holding.empty()) {
                    //If the leader isn't holding anybody.
                    switch_successful =
                        curLeaderPtr->group->changeStandbyType(
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
                        switch_successful =
                            curLeaderPtr->group->changeStandbyType(
                                action.actionTypeId == PLAYER_ACTION_TYPE_PREV_TYPE
                            );
                            
                        if(
                            !switch_successful ||
                            curLeaderPtr->group->curStandbyType ==
                            starting_subgroup_type
                        ) {
                            //Reached around back to the first subgroup...
                            switch_successful = false;
                            finish = true;
                            
                        } else {
                            //Switched to a new subgroup.
                            updateClosestGroupMembers();
                            if(!closestGroupMemberDistant) {
                                finish = true;
                            }
                            
                        }
                        
                    } while(!finish);
                    
                    if(switch_successful) {
                        curLeaderPtr->swapHeldPikmin(
                            closestGroupMember[BUBBLE_RELATION_CURRENT]
                        );
                    }
                }
                
                if(switch_successful) {
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
                !is_down ||
                !curLeaderPtr ||
                curLeaderPtr->holding.empty() ||
                curLeaderPtr->holding[0]->type->category->id !=
                MOB_CATEGORY_PIKMIN
            ) {
                return;
            }
            
            Pikmin* held_p_ptr = (Pikmin*) curLeaderPtr->holding[0];
            
            Pikmin* closest_members[N_MATURITIES];
            Distance closest_dists[N_MATURITIES];
            for(size_t m = 0; m < N_MATURITIES; m++) {
                closest_members[m] = nullptr;
            }
            
            for(size_t m = 0; m < curLeaderPtr->group->members.size(); m++) {
                Mob* m_ptr = curLeaderPtr->group->members[m];
                if(m_ptr->type != held_p_ptr->type) continue;
                
                Pikmin* p_ptr = (Pikmin*) m_ptr;
                if(p_ptr->maturity == held_p_ptr->maturity) continue;
                
                Distance d(curLeaderPtr->pos, p_ptr->pos);
                if(
                    !closest_members[p_ptr->maturity] ||
                    d < closest_dists[p_ptr->maturity]
                ) {
                    closest_members[p_ptr->maturity] = p_ptr;
                    closest_dists[p_ptr->maturity] = d;
                }
                
            }
            
            size_t next_maturity = held_p_ptr->maturity;
            Mob* new_pikmin = nullptr;
            bool finished = false;
            do {
                next_maturity =
                    (size_t) sumAndWrap(
                        (int) next_maturity,
                        (
                            action.actionTypeId ==
                            PLAYER_ACTION_TYPE_NEXT_MATURITY ? 1 : -1
                        ),
                        N_MATURITIES
                    );
                    
                //Back to the start?
                if(next_maturity == held_p_ptr->maturity) break;
                
                if(!closest_members[next_maturity]) continue;
                
                new_pikmin = closest_members[next_maturity];
                finished = true;
                
            } while(!finished);
            
            if(new_pikmin) {
                curLeaderPtr->swapHeldPikmin(new_pikmin);
            }
            
            break;
            
        }
        default: {
            break;
        }
        }
        
    } else if(msgBox) {
    
        //Displaying a message.
        if(action.actionTypeId == PLAYER_ACTION_TYPE_THROW && is_down) {
            msgBox->advance();
        } else if(action.actionTypeId == PLAYER_ACTION_TYPE_PAUSE && is_down) {
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
            leaderMovement.right = action.value;
            break;
        } case PLAYER_ACTION_TYPE_LEFT: {
            leaderMovement.left = action.value;
            break;
        } case PLAYER_ACTION_TYPE_UP: {
            leaderMovement.up = action.value;
            break;
        } case PLAYER_ACTION_TYPE_DOWN: {
            leaderMovement.down = action.value;
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
            cursorMovement.right = action.value;
            break;
        } case PLAYER_ACTION_TYPE_CURSOR_LEFT: {
            cursorMovement.left = action.value;
            break;
        } case PLAYER_ACTION_TYPE_CURSOR_UP: {
            cursorMovement.up = action.value;
            break;
        } case PLAYER_ACTION_TYPE_CURSOR_DOWN: {
            cursorMovement.down = action.value;
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
            swarmMovement.right = action.value;
            break;
        } case PLAYER_ACTION_TYPE_GROUP_LEFT: {
            swarmMovement.left = action.value;
            break;
        } case PLAYER_ACTION_TYPE_GROUP_UP: {
            swarmMovement.up = action.value;
            break;
        } case PLAYER_ACTION_TYPE_GROUP_DOWN: {
            swarmMovement.down = action.value;
            break;
        } default: {
            break;
        }
        }
        
        break;
        
    } case PLAYER_ACTION_TYPE_GROUP_CURSOR: {

        swarmCursor = is_down;
        
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
    const vector<int> actions_allowed_during_interludes {
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
                actions_allowed_during_interludes, action.actionTypeId
            )
        ) {
            return true;
        }
    }
    
    return false;
}
