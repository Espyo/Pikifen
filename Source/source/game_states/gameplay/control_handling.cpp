/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Control handling.
 */

#include "gameplay.h"

#include "../../game.h"


/* ----------------------------------------------------------------------------
 * Handles a player action.
 * action:
 *   Data about the action.
 */
void gameplay_state::handle_player_action(const player_action &action) {

    if(!ready_for_input || !is_input_allowed) return;
    if(cur_interlude != INTERLUDE_NONE) return;
    
    bool is_down = (action.value >= 0.5);
    
    //Before we do the actions, we'll tell the leader object
    //it's recieved an input, which will trigger an event.
    if(cur_leader_ptr) {
        cur_leader_ptr->fsm.run_event(
            MOB_EV_INPUT_RECEIVED,
            (void*) &action
        );
    }
    
    if(!msg_box && !onion_menu && !pause_menu) {
    
        switch(action.action_type_id) {
        case PLAYER_ACTION_THROW: {
    
            /*******************
            *             .-.  *
            *   Throw    /   O *
            *           &      *
            *******************/
            
            if(is_down) { //Button press.
            
                bool done = false;
                
                //Check if the player wants to cancel auto-throw.
                if(
                    cur_leader_ptr &&
                    game.options.auto_throw_mode == AUTO_THROW_TOGGLE &&
                    cur_leader_ptr->auto_throwing
                ) {
                    cur_leader_ptr->stop_auto_throwing();
                    done = true;
                }
                
                //Check if the leader should heal themselves on the ship.
                if(
                    !done &&
                    cur_leader_ptr &&
                    close_to_ship_to_heal
                ) {
                    close_to_ship_to_heal->heal_leader(cur_leader_ptr);
                    done = true;
                }
                
                //Check if the leader should pluck a Pikmin.
                if(
                    !done &&
                    cur_leader_ptr &&
                    close_to_pikmin_to_pluck
                ) {
                    cur_leader_ptr->fsm.run_event(
                        LEADER_EV_GO_PLUCK,
                        (void*)close_to_pikmin_to_pluck
                    );
                    done = true;
                }
                
                //Now check if the leader should open an Onion's menu.
                if(
                    !done &&
                    cur_leader_ptr &&
                    close_to_nest_to_open
                ) {
                    onion_menu = new onion_menu_struct(
                        close_to_nest_to_open,
                        cur_leader_ptr
                    );
                    hud->gui.start_animation(
                        GUI_MANAGER_ANIM_IN_TO_OUT,
                        GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
                    );
                    paused = true;
                    
                    //TODO replace with a better solution.
                    cur_leader_ptr->fsm.run_event(LEADER_EV_STOP_WHISTLE);
                    
                    done = true;
                }
                
                //Now check if the leader should interact with an interactable.
                if(
                    !done &&
                    cur_leader_ptr &&
                    close_to_interactable_to_use
                ) {
                    string msg = "interact";
                    cur_leader_ptr->send_message(
                        close_to_interactable_to_use, msg
                    );
                    done = true;
                }
                
                //Now check if the leader should grab a Pikmin.
                if(
                    !done &&
                    cur_leader_ptr &&
                    cur_leader_ptr->holding.empty() &&
                    cur_leader_ptr->group->cur_standby_type &&
                    !closest_group_member_distant
                ) {
                    switch (game.options.auto_throw_mode) {
                    case AUTO_THROW_OFF: {
                        done = grab_closest_group_member();
                        break;
                    } case AUTO_THROW_HOLD:
                    case AUTO_THROW_TOGGLE: {
                        cur_leader_ptr->start_auto_throwing();
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
                    cur_leader_ptr
                ) {
                    cur_leader_ptr->fsm.run_event(LEADER_EV_PUNCH);
                    done = true;
                }
                
            } else { //Button release.
            
                if(cur_leader_ptr) {
                    switch (game.options.auto_throw_mode) {
                    case AUTO_THROW_OFF: {
                        cur_leader_ptr->queue_throw();
                        break;
                    } case AUTO_THROW_HOLD: {
                        cur_leader_ptr->stop_auto_throwing();
                        break;
                    } case AUTO_THROW_TOGGLE: {
                        break;
                    } default: {
                        break;
                    }
                    }
                }
                
            }
            
            break;
            
        } case PLAYER_ACTION_WHISTLE: {
    
            /********************
            *              .--= *
            *   Whistle   ( @ ) *
            *              '-'  *
            ********************/
            
            if(is_down) {
                //Button pressed.
                
                if(cur_leader_ptr) {
                    mob_event* cancel_ev =
                        cur_leader_ptr->fsm.get_event(LEADER_EV_CANCEL);
                        
                    if(cancel_ev) {
                        //Cancel auto-pluck, lying down, etc.
                        cancel_ev->run(cur_leader_ptr);
                    } else {
                        //Start whistling.
                        cur_leader_ptr->fsm.run_event(LEADER_EV_START_WHISTLE);
                    }
                }
                
            } else {
                //Button released.
                
                if(cur_leader_ptr) {
                    cur_leader_ptr->fsm.run_event(LEADER_EV_STOP_WHISTLE);
                }
                
            }
            
            break;
            
        } case PLAYER_ACTION_NEXT_LEADER:
        case PLAYER_ACTION_PREV_LEADER: {
    
            /******************************
            *                    \O/  \O/ *
            *   Switch leader     | -> |  *
            *                    / \  / \ *
            ******************************/
            
            if(!is_down) return;
            
            change_to_next_leader(
                action.action_type_id == PLAYER_ACTION_NEXT_LEADER,
                false, false
            );
            
            break;
            
        } case PLAYER_ACTION_DISMISS: {
    
            /***********************
            *             \O/ / *  *
            *   Dismiss    |   - * *
            *             / \ \ *  *
            ***********************/
            
            if(!is_down) return;
            
            if(cur_leader_ptr) {
                cur_leader_ptr->fsm.run_event(LEADER_EV_DISMISS);
            }
            
            break;
            
        } case PLAYER_ACTION_PAUSE: {
    
            /********************
            *           +-+ +-+ *
            *   Pause   | | | | *
            *           +-+ +-+ *
            ********************/
            
            if(!is_down) return;
            
            pause_menu = new pause_menu_struct();
            paused = true;
            hud->gui.start_animation(
                GUI_MANAGER_ANIM_IN_TO_OUT,
                GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
            );
            
            //TODO replace with a better solution.
            if(cur_leader_ptr) {
                cur_leader_ptr->fsm.run_event(LEADER_EV_STOP_WHISTLE);
            }
            
            break;
            
        } case PLAYER_ACTION_USE_SPRAY_1: {
    
            /*******************
            *             +=== *
            *   Sprays   (   ) *
            *             '-'  *
            *******************/
            
            if(!is_down) return;
            
            if(cur_leader_ptr) {
                if(
                    game.spray_types.size() == 1 ||
                    game.spray_types.size() == 2
                ) {
                    size_t spray_nr = 0;
                    cur_leader_ptr->fsm.run_event(
                        LEADER_EV_SPRAY, (void*) &spray_nr
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_USE_SPRAY_2: {
    
            if(!is_down) return;
            
            if(cur_leader_ptr) {
                if(game.spray_types.size() == 2) {
                    size_t spray_nr = 1;
                    cur_leader_ptr->fsm.run_event(
                        LEADER_EV_SPRAY, (void*) &spray_nr
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_NEXT_SPRAY:
        case PLAYER_ACTION_PREV_SPRAY: {
    
            if(!is_down) return;
            
            if(cur_leader_ptr) {
                if(game.spray_types.size() > 2) {
                    selected_spray =
                        sum_and_wrap(
                            (int) selected_spray,
                            action.action_type_id ==
                            PLAYER_ACTION_NEXT_SPRAY ? +1 : -1,
                            (int) game.spray_types.size()
                        );
                    game.states.gameplay->hud->
                    spray_1_amount->start_juice_animation(
                        gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_USE_SPRAY: {
    
            if(!is_down) return;
            
            if(cur_leader_ptr) {
                if(game.spray_types.size() > 2) {
                    cur_leader_ptr->fsm.run_event(
                        LEADER_EV_SPRAY,
                        (void*) &selected_spray
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_CHANGE_ZOOM: {
    
            /***************
            *           _  *
            *   Zoom   (_) *
            *          /   *
            ***************/
            
            if(!is_down) return;
            
            if(game.cam.target_zoom < game.options.zoom_mid_level) {
                game.cam.target_zoom = game.config.zoom_max_level;
            } else if(game.cam.target_zoom > game.options.zoom_mid_level) {
                game.cam.target_zoom = game.options.zoom_mid_level;
            } else {
                if(game.options.zoom_mid_level == game.config.zoom_min_level) {
                    game.cam.target_zoom = game.config.zoom_max_level;
                } else {
                    game.cam.target_zoom = game.config.zoom_min_level;
                }
            }
            
            game.audio.create_global_sfx_source(game.sys_assets.sfx_camera);
            
            break;
            
        } case PLAYER_ACTION_ZOOM_IN:
        case PLAYER_ACTION_ZOOM_OUT: {
    
            if(
                game.cam.target_zoom >= game.config.zoom_max_level &&
                action.action_type_id == PLAYER_ACTION_ZOOM_IN
            ) {
                return;
            }
            
            if(
                game.cam.target_zoom <= game.config.zoom_min_level &&
                action.action_type_id == PLAYER_ACTION_ZOOM_OUT
            ) {
                return;
            }
            
            float floored_pos = floor(action.value);
            
            if(action.action_type_id == PLAYER_ACTION_ZOOM_IN) {
                game.cam.target_zoom = game.cam.target_zoom + 0.1 * floored_pos;
            } else {
                game.cam.target_zoom = game.cam.target_zoom - 0.1 * floored_pos;
            }
            
            if(game.cam.target_zoom > game.config.zoom_max_level) {
                game.cam.target_zoom = game.config.zoom_max_level;
            }
            if(game.cam.target_zoom < game.config.zoom_min_level) {
                game.cam.target_zoom = game.config.zoom_min_level;
            }
            
            sfx_source_config_struct cam_sfx_config;
            cam_sfx_config.stack_mode = SFX_STACK_NEVER;
            game.audio.create_global_sfx_source(
                game.sys_assets.sfx_camera,
                cam_sfx_config
            );
            
            break;
            
        } case PLAYER_ACTION_LIE_DOWN: {
    
            /**********************
            *                     *
            *   Lie down  -()/__/ *
            *                     *
            **********************/
            
            if(!is_down) return;
            
            if(cur_leader_ptr) {
                cur_leader_ptr->fsm.run_event(LEADER_EV_LIE_DOWN);
            }
            
            break;
            
        } case PLAYER_ACTION_NEXT_TYPE:
        case PLAYER_ACTION_PREV_TYPE: {
    
            /****************************
            *                     -->   *
            *   Switch type   <( )> (o) *
            *                           *
            ****************************/
            
            if(!is_down) return;
            
            if(cur_leader_ptr) {
                if(cur_leader_ptr->group->members.empty()) return;
                
                subgroup_type* starting_subgroup_type =
                    cur_leader_ptr->group->cur_standby_type;
                    
                bool switch_successful;
                
                if(cur_leader_ptr->holding.empty()) {
                    //If the leader isn't holding anybody.
                    switch_successful =
                        cur_leader_ptr->group->change_standby_type(
                            action.action_type_id == PLAYER_ACTION_PREV_TYPE
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
                            cur_leader_ptr->group->change_standby_type(
                                action.action_type_id == PLAYER_ACTION_PREV_TYPE
                            );
                            
                        if(
                            !switch_successful ||
                            cur_leader_ptr->group->cur_standby_type ==
                            starting_subgroup_type
                        ) {
                            //Reached around back to the first subgroup...
                            switch_successful = false;
                            finish = true;
                            
                        } else {
                            //Switched to a new subgroup.
                            update_closest_group_members();
                            if(!closest_group_member_distant) {
                                finish = true;
                            }
                            
                        }
                        
                    } while(!finish);
                    
                    if(switch_successful) {
                        cur_leader_ptr->swap_held_pikmin(
                            closest_group_member[BUBBLE_CURRENT]
                        );
                    }
                }
                
                if(switch_successful) {
                    game.audio.create_global_sfx_source(
                        game.sys_assets.sfx_switch_pikmin
                    );
                }
            }
            
            break;
            
        } case PLAYER_ACTION_NEXT_MATURITY:
        case PLAYER_ACTION_PREV_MATURITY: {
    
            /**********************************
            *                      V  -->  *  *
            *   Switch maturity    |       |  *
            *                     ( )     ( ) *
            **********************************/
            
            if(
                !is_down ||
                !cur_leader_ptr ||
                cur_leader_ptr->holding.empty() ||
                cur_leader_ptr->holding[0]->type->category->id !=
                MOB_CATEGORY_PIKMIN
            ) {
                return;
            }
            
            pikmin* held_p_ptr = (pikmin*) cur_leader_ptr->holding[0];
            
            pikmin* closest_members[N_MATURITIES];
            dist closest_dists[N_MATURITIES];
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                closest_members[m] = NULL;
            }
            
            for(size_t m = 0; m < cur_leader_ptr->group->members.size(); ++m) {
                mob* m_ptr = cur_leader_ptr->group->members[m];
                if(m_ptr->type != held_p_ptr->type) continue;
                
                pikmin* p_ptr = (pikmin*) m_ptr;
                if(p_ptr->maturity == held_p_ptr->maturity) continue;
                
                dist d(cur_leader_ptr->pos, p_ptr->pos);
                if(
                    !closest_members[p_ptr->maturity] ||
                    d < closest_dists[p_ptr->maturity]
                ) {
                    closest_members[p_ptr->maturity] = p_ptr;
                    closest_dists[p_ptr->maturity] = d;
                }
                
            }
            
            size_t next_maturity = held_p_ptr->maturity;
            mob* new_pikmin = NULL;
            bool finished = false;
            do {
                next_maturity =
                    (size_t) sum_and_wrap(
                        (int) next_maturity,
                        (
                            action.action_type_id ==
                            PLAYER_ACTION_NEXT_MATURITY ? 1 : -1
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
                cur_leader_ptr->swap_held_pikmin(new_pikmin);
            }
            
            break;
            
        }
        default: {
            break;
        }
        }
        
    } else if(msg_box) {
    
        //Displaying a message.
        if(
            (
                action.action_type_id == PLAYER_ACTION_THROW ||
                action.action_type_id == PLAYER_ACTION_PAUSE
            ) && is_down
        ) {
            msg_box->advance();
        }
        
    }
    //Some inputs we don't want to ignore even if we're in a menu.
    //Those go here.
    switch (action.action_type_id) {
    case PLAYER_ACTION_RIGHT:
    case PLAYER_ACTION_UP:
    case PLAYER_ACTION_LEFT:
    case PLAYER_ACTION_DOWN: {
        /*******************
        *               O_ *
        *   Move   --->/|  *
        *              V > *
        *******************/
        
        switch(action.action_type_id) {
        case PLAYER_ACTION_RIGHT: {
            leader_movement.right = action.value;
            break;
        } case PLAYER_ACTION_LEFT: {
            leader_movement.left = action.value;
            break;
        } case PLAYER_ACTION_UP: {
            leader_movement.up = action.value;
            break;
        } case PLAYER_ACTION_DOWN: {
            leader_movement.down = action.value;
            break;
        } default: {
            break;
        }
        }
        
        break;
        
    } case PLAYER_ACTION_CURSOR_RIGHT:
    case PLAYER_ACTION_CURSOR_UP:
    case PLAYER_ACTION_CURSOR_LEFT:
    case PLAYER_ACTION_CURSOR_DOWN: {
        /********************
        *             .-.   *
        *   Cursor   ( = )> *
        *             '-'   *
        ********************/
        
        switch(action.action_type_id) {
        case PLAYER_ACTION_CURSOR_RIGHT: {
            cursor_movement.right = action.value;
            break;
        } case PLAYER_ACTION_CURSOR_LEFT: {
            cursor_movement.left = action.value;
            break;
        } case PLAYER_ACTION_CURSOR_UP: {
            cursor_movement.up = action.value;
            break;
        } case PLAYER_ACTION_CURSOR_DOWN: {
            cursor_movement.down = action.value;
            break;
        } default: {
            break;
        }
        }
        
        break;
        
    } case PLAYER_ACTION_GROUP_RIGHT:
    case PLAYER_ACTION_GROUP_UP:
    case PLAYER_ACTION_GROUP_LEFT:
    case PLAYER_ACTION_GROUP_DOWN: {
        /******************
        *            ***  *
        *   Group   ****O *
        *            ***  *
        ******************/
        
        switch(action.action_type_id) {
        case PLAYER_ACTION_GROUP_RIGHT: {
            swarm_movement.right = action.value;
            break;
        } case PLAYER_ACTION_GROUP_LEFT: {
            swarm_movement.left = action.value;
            break;
        } case PLAYER_ACTION_GROUP_UP: {
            swarm_movement.up = action.value;
            break;
        } case PLAYER_ACTION_GROUP_DOWN: {
            swarm_movement.down = action.value;
            break;
        } default: {
            break;
        }
        }
        
        break;
        
    } case PLAYER_ACTION_GROUP_CURSOR: {

        swarm_cursor = is_down;
        
        break;
        
    } default: {
        break;
    }
    }
    
}
