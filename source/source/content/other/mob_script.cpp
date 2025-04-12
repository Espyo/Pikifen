/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob script classes and
 * related functions.
 */

#include <algorithm>

#include "mob_script.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob_script/gen_mob_fsm.h"
#include "../mob_type/mob_type.h"
#include "../mob/mob.h"
#include "mob_script_action.h"
#include "particle.h"


/**
 * @brief Creates a new action call for the current event, one that changes
 * the mob's state to something else.
 *
 * @param new_state State to change to.
 */
void EasyFsmCreator::changeState(const string &new_state) {
    curEvent->actions.push_back(new MobActionCall(MOB_ACTION_SET_STATE));
    curEvent->actions.back()->args.push_back(new_state);
    curEvent->actions.back()->argIsVar.push_back(false);
}


/**
 * @brief Finishes the event that is currently under construction, if any.
 */
void EasyFsmCreator::commitEvent() {
    if(!curEvent) return;
    curEvent = nullptr;
}


/**
 * @brief Finishes the state that is currently under construction, if any.
 */
void EasyFsmCreator::commitState() {
    if(!curState) return;
    commitEvent();
    curState = nullptr;
}


/**
 * @brief Finishes any event or state under construction and returns the
 * final vector of states.
 *
 * @return The states.
 */
vector<MobState*> EasyFsmCreator::finish() {
    commitEvent();
    commitState();
    sort(
        states.begin(), states.end(),
    [] (const MobState * ms1, const MobState * ms2) -> bool {
        return ms1->id < ms2->id;
    }
    );
    return states;
}


/**
 * @brief Finishes the previous event, if any, creates a new event for the
 * current state, and starts tracking for the creation of its actions.
 *
 * @param type Type of event.
 */
void EasyFsmCreator::newEvent(const MOB_EV type) {
    commitEvent();
    curEvent = new MobEvent(type);
    curState->events[type] = curEvent;
}


/**
 * @brief Finishes the previous state, if any, creates a new state,
 * and starts tracking for the creation of its events.
 *
 * @param name Name of the state.
 * @param id Its ID.
 */
void EasyFsmCreator::newState(const string &name, size_t id) {
    commitState();
    curState = new MobState(name, id);
    states.push_back(curState);
}


/**
 * @brief Creates a new action for the current event, one that
 * runs some custom code.
 *
 * @param code Function with said code.
 */
void EasyFsmCreator::run(custom_action_code_t code) {
    curEvent->actions.push_back(new MobActionCall(code));
}


/**
 * @brief Constructs a new hitbox interaction object.
 *
 * @param mob2 The other mob.
 * @param h1 The current mob's hitbox.
 * @param h2 The other mob's hitbox.
 */
HitboxInteraction::HitboxInteraction(
    Mob* mob2, Hitbox* h1, Hitbox* h2
) {
    this->mob2 = mob2;
    this->h1 = h1;
    this->h2 = h2;
}


/**
 * @brief Constructs a new mob event object given a data node.
 *
 * @param node The data node.
 * @param actions Its actions.
 */
MobEvent::MobEvent(
    const DataNode* node, const vector<MobActionCall*> &actions
) :
    actions(actions) {
    
#define r(name, number) \
    else if(n == (name)) type = (number)
    
    const string &n = node->name;
    if(n == "on_enter") type =          MOB_EV_ON_ENTER;
    r("on_leave",                       MOB_EV_ON_LEAVE);
    r("on_tick",                        MOB_EV_ON_TICK);
    r("on_ready",                       MOB_EV_ON_READY);
    r("on_animation_end",               MOB_EV_ANIMATION_END);
    r("on_damage",                      MOB_EV_DAMAGE);
    r("on_far_from_home",               MOB_EV_FAR_FROM_HOME);
    r("on_finish_receiving_delivery",   MOB_EV_FINISHED_RECEIVING_DELIVERY);
    r("on_focus_off_reach",             MOB_EV_FOCUS_OFF_REACH);
    r("on_frame_signal",                MOB_EV_FRAME_SIGNAL);
    r("on_held",                        MOB_EV_HELD);
    r("on_hitbox_touch_eat",            MOB_EV_HITBOX_TOUCH_EAT);
    r("on_hitbox_touch_a_n",            MOB_EV_HITBOX_TOUCH_A_N);
    r("on_hitbox_touch_n_n",            MOB_EV_HITBOX_TOUCH_N_N);
    r("on_input_received",              MOB_EV_INPUT_RECEIVED);
    r("on_itch",                        MOB_EV_ITCH);
    r("on_land",                        MOB_EV_LANDED);
    r("on_leave_hazard",                MOB_EV_LEFT_HAZARD);
    r("on_object_in_reach",             MOB_EV_OBJECT_IN_REACH);
    r("on_opponent_in_reach",           MOB_EV_OPPONENT_IN_REACH);
    r("on_pikmin_land",                 MOB_EV_THROWN_PIKMIN_LANDED);
    r("on_receive_message",             MOB_EV_RECEIVE_MESSAGE);
    r("on_released",                    MOB_EV_RELEASED);
    r("on_reach_destination",           MOB_EV_REACHED_DESTINATION);
    r("on_start_receiving_delivery",    MOB_EV_STARTED_RECEIVING_DELIVERY);
    r("on_swallowed",                   MOB_EV_SWALLOWED);
    r("on_timer",                       MOB_EV_TIMER);
    r("on_touch_hazard",                MOB_EV_TOUCHED_HAZARD);
    r("on_touch_object",                MOB_EV_TOUCHED_OBJECT);
    r("on_touch_opponent",              MOB_EV_TOUCHED_OPPONENT);
    r("on_touch_wall",                  MOB_EV_TOUCHED_WALL);
    r("on_weight_added",                MOB_EV_WEIGHT_ADDED);
    r("on_weight_removed",              MOB_EV_WEIGHT_REMOVED);
    
    else {
        type = MOB_EV_UNKNOWN;
        game.errors.report("Unknown script event name \"" + n + "\"!", node);
    }
    
    for(size_t a = 0; a < this->actions.size(); a++) {
        this->actions[a]->parentEvent = (MOB_EV) type;
    }
}


/**
 * @brief Constructs a new mob event object.
 *
 * @param t The event type.
 * @param a Its actions.
 */
MobEvent::MobEvent(const MOB_EV t, const vector<MobActionCall*> &a) :
    type(t),
    actions(a) {
    
}


/**
 * @brief Runs a mob event. Basically runs all actions within.
 *
 * @param m The mob.
 * @param custom_data_1 Custom argument #1 to pass to the code.
 * @param custom_data_2 Custom argument #2 to pass to the code.
 */
void MobEvent::run(Mob* m, void* custom_data_1, void* custom_data_2) {
    if(m->parent && m->parent->relay_events) {
        m->parent->m->fsm.runEvent(type, custom_data_1, custom_data_2);
        if(!m->parent->handle_events) {
            return;
        }
    }
    
    for(size_t a = 0; a < actions.size(); a++) {
    
        switch(actions[a]->action->type) {
        case MOB_ACTION_IF: {
            //If statement. Look out for its return value, and
            //change the flow accordingly.
            
            if(!actions[a]->run(m, custom_data_1, custom_data_2)) {
                //If it returned true, execution continues as normal, but
                //if it returned false, skip to the "else" or "end if" actions.
                size_t next_a = a + 1;
                size_t depth = 0;
                for(; next_a < actions.size(); next_a++) {
                    if(
                        actions[next_a]->action->type == MOB_ACTION_IF
                    ) {
                        depth++;
                    } else if(
                        actions[next_a]->action->type == MOB_ACTION_ELSE
                    ) {
                        if(depth == 0) break;
                    } else if(
                        actions[next_a]->action->type == MOB_ACTION_END_IF
                    ) {
                        if(depth == 0) break;
                        else depth--;
                    }
                }
                a = next_a;
                
            }
            
            break;
            
        } case MOB_ACTION_ELSE: {
            //If we actually managed to read an "else", that means we were
            //running through the normal execution of a "then" section.
            //Jump to the "end if".
            size_t next_a = a + 1;
            size_t depth = 0;
            for(; next_a < actions.size(); next_a++) {
                if(actions[next_a]->action->type == MOB_ACTION_IF) {
                    depth++;
                } else if(actions[next_a]->action->type == MOB_ACTION_END_IF) {
                    if(depth == 0) break;
                    else depth--;
                }
            }
            a = next_a;
            
            break;
            
        } case MOB_ACTION_GOTO: {
            //Find the label that matches.
            for(size_t a2 = 0; a2 < actions.size(); a2++) {
                if(actions[a2]->action->type == MOB_ACTION_LABEL) {
                    if(actions[a]->args[0] == actions[a2]->args[0]) {
                        a = a2;
                        break;
                    }
                }
            }
            break;
            
        } case MOB_ACTION_END_IF:
        case MOB_ACTION_LABEL: {
            //Nothing to do.
            break;
            
        } default: {
            //Normal action.
            actions[a]->run(m, custom_data_1, custom_data_2);
            //If the state got changed, jump out.
            if(actions[a]->action->type == MOB_ACTION_SET_STATE) return;
            
            break;
        }
        }
    }
}


/**
 * @brief Constructs a new mob FSM object.
 *
 * @param m The mob this FSM belongs to.
 */
MobFsm::MobFsm(Mob* m) {

    if(!m) return;
    this->m = m;
}


/**
 * @brief Returns a pointer to an event of the given type in the current state,
 * if it exists.
 *
 * @param type The event's type.
 * @return The event.
 */
MobEvent* MobFsm::getEvent(const MOB_EV type) const {
    return curState->events[type];
}


/**
 * @brief Returns the index of the specified state.
 *
 * @param name The state's name.
 * @return The index, or INVALID if it doesn't exist.
 */
size_t MobFsm::getStateIdx(const string &name) const {
    for(size_t s = 0; s < m->type->states.size(); s++) {
        if(m->type->states[s]->name == name) {
            return s;
        }
    }
    return INVALID;
}


/**
 * @brief Runs an event in the current state, if it exists.
 *
 * @param type The event's type.
 * @param custom_data_1 Custom argument #1 to pass to the code.
 * @param custom_data_2 Custom argument #2 to pass to the code.
 */
void MobFsm::runEvent(
    const MOB_EV type, void* custom_data_1, void* custom_data_2
) {
    MobEvent* e = getEvent(type);
    if(e) {
        e->run(m, custom_data_1, custom_data_2);
    } else {
    
#ifdef DEBUG_FSM
        cout <<
             "Missing event on run_event() - Mob " <<
             m << ", event " << type << ", state " <<
             (this->curState ? this->curState->name : "[None]") <<
             endl;
#endif
             
        return;
    }
}


/**
 * @brief Changes the FSM to use a different state.
 *
 * @param new_state The state to change to.
 * @param info1 Data to pass on to the code after the state change.
 * This data comes from the event that started all of this.
 * @param info2 Same as info1, but a second variable.
 * @return Whether it succeeded.
 */
bool MobFsm::setState(size_t new_state, void* info1, void* info2) {

    //Run the code to leave the current state.
    if(curState) {
        for(unsigned char p = STATE_HISTORY_SIZE - 1; p > 0; --p) {
            prevStateNames[p] = prevStateNames[p - 1];
        }
        prevStateNames[0] = curState->name;
        runEvent(MOB_EV_ON_LEAVE, info1, info2);
    }
    
    //Uncomment this to be notified about state changes on stdout.
    /*if(curState) {
        cout << "State " << curState->name << " -> "
        << m->type->states[new_state]->name << "\n";
    }*/
    
    if(new_state < m->type->states.size() && new_state != INVALID) {
        //Switch states.
        curState = m->type->states[new_state];
        
        //Run the code to enter the new state.
        runEvent(MOB_EV_ON_ENTER, info1, info2);
        
        return true;
    }
    
    return false;
    
}


/**
 * @brief Constructs a new mob state object.
 *
 * @param name The state's name.
 */
MobState::MobState(const string &name) :
    name(name) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; e++) {
        events[e] = nullptr;
    }
}


/**
 * @brief Constructs a new mob state object.
 *
 * @param name The state's name.
 * @param evs Its events.
 */
MobState::MobState(const string &name, MobEvent* evs[N_MOB_EVENTS]) :
    name(name) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; e++) {
        events[e] = evs[e];
    }
}


/**
 * @brief Constructs a new mob state object.
 *
 * @param name The state's name.
 * @param id Its ID, for sorting on the vector of states.
 */
MobState::MobState(const string &name, size_t id) :
    name(name),
    id(id) {
    
    for(size_t e = 0; e < N_MOB_EVENTS; e++) {
        events[e] = nullptr;
    }
}


/**
 * @brief Returns a pointer to an event of the given type in the state,
 * if it exists.
 *
 * @param type The event's type.
 * @return The event.
 */
MobEvent* MobState::getEvent(const MOB_EV type) const {
    return events[type];
}


/**
 * @brief Fixes some things in the list of states.
 * For instance, state-switching actions that use a name instead of an index.
 *
 * @param states The vector of states.
 * @param starting_state Name of the starting state for the mob.
 * @param mt Mob type these states belong to.
 * @return The index of the starting state.
 */
size_t fixStates(
    vector<MobState*> &states, const string &starting_state, const MobType* mt
) {
    size_t starting_state_idx = INVALID;
    
    //Fix actions that change the state that are using a string.
    for(size_t s = 0; s < states.size(); s++) {
        MobState* state = states[s];
        if(state->name == starting_state) starting_state_idx = s;
        
        for(size_t e = 0; e < N_MOB_EVENTS; e++) {
            MobEvent* ev = state->events[e];
            if(!ev) continue;
            
            for(size_t a = 0; a < ev->actions.size(); a++) {
                MobActionCall* call = ev->actions[a];
                
                if(call->action->type == MOB_ACTION_SET_STATE) {
                    string state_name = call->args[0];
                    size_t state_idx = 0;
                    bool found_state = false;
                    
                    if(isNumber(state_name)) continue;
                    
                    for(; state_idx < states.size(); state_idx++) {
                        if(states[state_idx]->name == state_name) {
                            found_state = true;
                            break;
                        }
                    }
                    
                    if(!found_state) {
                        state_idx = INVALID;
                        game.errors.report(
                            "State \"" + state->name +
                            "\" of the mob category \"" + mt->category->name +
                            "\" has an action to switch to an "
                            "unknown state: \"" + state_name + "\"!",
                            nullptr
                        );
                    }
                    
                    call->args[0] = i2s(state_idx);
                    
                }
            }
        }
    }
    return starting_state_idx;
}


/**
 * @brief Loads the states from the script and global events data nodes.
 *
 * @param mt The type of mob the states are going to.
 * @param script_node The data node containing the mob's script.
 * @param global_node The data node containing global events.
 * @param out_states The loaded states are returned into this vector.
 */
void loadScript(
    MobType* mt, DataNode* script_node, DataNode* global_node,
    vector<MobState*>* out_states
) {
    size_t n_new_states = script_node->getNrOfChildren();
    
    //Let's save the states now, so that the state switching events
    //can know what numbers the events they need correspond to.
    for(size_t s = 0; s < n_new_states; s++) {
        DataNode* state_node = script_node->getChild(s);
        bool skip = false;
        for(size_t s2 = 0; s2 < out_states->size(); s2++) {
            if((*out_states)[s2]->name == state_node->name) {
                //Already exists, probably hardcoded. Skip this.
                skip = true;
                continue;
            }
        }
        if(!skip) {
            out_states->push_back(new MobState(state_node->name));
        }
    }
    
    for(size_t s = 0; s < out_states->size(); s++) {
        MobState* state_ptr = (*out_states)[s];
        DataNode* state_node = script_node->getChildByName(state_ptr->name);
        loadState(mt, state_node, global_node, state_ptr);
        state_ptr->id = s;
    }
    
    fixStates(*out_states, "", mt);
}


/**
 * @brief Loads a state from the script and global events data nodes.
 *
 * @param mt The type of mob the states are going to.
 * @param state_node The state's data node.
 * @param global_node The data node containing global events.
 * @param state_ptr Pointer to the state to load.
 */
void loadState(
    MobType* mt, DataNode* state_node, DataNode* global_node,
    MobState* state_ptr
) {
    size_t n_events = state_node->getNrOfChildren();
    size_t n_global_events = global_node->getNrOfChildren();
    if(n_events + n_global_events == 0) return;
    
    //Read the events.
    vector<MobEvent*> new_events;
    vector<bitmask_8_t> new_event_settings;
    
    for(size_t e = 0; e < n_events; e++) {
        DataNode* event_node = state_node->getChild(e);
        vector<MobActionCall*> actions;
        bitmask_8_t settings;
        
        loadActions(mt, event_node, &actions, &settings);
        
        new_events.push_back(new MobEvent(event_node, actions));
        new_event_settings.push_back(settings);
        
        assertActions(actions, event_node);
    }
    
    //Load global events.
    vector<MobEvent*> global_events;
    vector<bitmask_8_t> global_event_settings;
    
    for(size_t e = 0; e < n_global_events; e++) {
        DataNode* event_node = global_node->getChild(e);
        vector<MobActionCall*> actions;
        bitmask_8_t settings;
        
        loadActions(mt, event_node, &actions, &settings);
        
        global_events.push_back(new MobEvent(event_node, actions));
        global_event_settings.push_back(settings);
        
        assertActions(actions, event_node);
    }
    
    //Insert global events into the state.
    for(size_t e = 0; e < global_events.size(); e++) {
        MobEvent* global_event = global_events[e];
        bitmask_8_t global_settings = global_event_settings[e];
        
        bool merged = false;
        for(size_t ne = 0; ne < n_events; ne++) {
            MobEvent* ev_ptr = new_events[ne];
            bitmask_8_t ev_settings = new_event_settings[ne];
            
            if(ev_ptr->type != global_event->type) continue;
            
            insertEventActions(
                ev_ptr,
                global_event->actions,
                hasFlag(
                    global_settings | ev_settings,
                    EVENT_LOAD_FLAG_GLOBAL_ACTIONS_AFTER
                )
            );
            merged = true;
            break;
        }
        if(merged) {
            delete global_event;
        } else {
            new_events.push_back(global_event);
            new_event_settings.push_back(global_settings);
        }
    }
    
    //Inject a damage event.
    if(!state_ptr->events[MOB_EV_HITBOX_TOUCH_N_A]) {
        vector<MobActionCall*> da_actions;
        da_actions.push_back(
            new MobActionCall(gen_mob_fsm::beAttacked)
        );
        new_events.push_back(
            new MobEvent(MOB_EV_HITBOX_TOUCH_N_A, da_actions)
        );
        new_event_settings.push_back(0);
    }
    
    //Inject a zero health event.
    if(
        state_node->name != mt->dyingStateName &&
        !state_ptr->events[MOB_EV_ZERO_HEALTH] &&
        find(
            mt->statesIgnoringDeath.begin(),
            mt->statesIgnoringDeath.end(),
            state_node->name
        ) == mt->statesIgnoringDeath.end() &&
        !mt->dyingStateName.empty()
    ) {
        vector<MobActionCall*> de_actions;
        de_actions.push_back(new MobActionCall(gen_mob_fsm::goToDyingState));
        new_events.push_back(new MobEvent(MOB_EV_ZERO_HEALTH, de_actions));
        new_event_settings.push_back(0);
    }
    
    //Inject a bottomless pit event.
    if(!state_ptr->events[MOB_EV_BOTTOMLESS_PIT]) {
        vector<MobActionCall*> bp_actions;
        bp_actions.push_back(
            new MobActionCall(gen_mob_fsm::fallDownPit)
        );
        new_events.push_back(
            new MobEvent(MOB_EV_BOTTOMLESS_PIT, bp_actions)
        );
        new_event_settings.push_back(0);
    }
    
    //Inject a spray touch event.
    if(
        !state_ptr->events[MOB_EV_TOUCHED_SPRAY] &&
        find(
            mt->statesIgnoringSpray.begin(),
            mt->statesIgnoringSpray.end(),
            state_node->name
        ) == mt->statesIgnoringSpray.end()
    ) {
        vector<MobActionCall*> s_actions;
        s_actions.push_back(
            new MobActionCall(gen_mob_fsm::touchSpray)
        );
        new_events.push_back(
            new MobEvent(MOB_EV_TOUCHED_SPRAY, s_actions)
        );
        new_event_settings.push_back(0);
    }
    
    //Inject a hazard event.
    if(
        !state_ptr->events[MOB_EV_TOUCHED_HAZARD] &&
        find(
            mt->statesIgnoringHazard.begin(),
            mt->statesIgnoringHazard.end(),
            state_node->name
        ) == mt->statesIgnoringHazard.end()
    ) {
        vector<MobActionCall*> s_actions;
        s_actions.push_back(
            new MobActionCall(gen_mob_fsm::touchHazard)
        );
        new_events.push_back(
            new MobEvent(MOB_EV_TOUCHED_HAZARD, s_actions)
        );
        new_event_settings.push_back(0);
    }
    
    //Connect all new events to the state.
    for(size_t e = 0; e < new_events.size(); e++) {
        MOB_EV ev_type = new_events[e]->type;
        
        if(state_ptr->events[ev_type]) {
            insertEventActions(
                state_ptr->events[ev_type],
                new_events[e]->actions,
                hasFlag(
                    new_event_settings[e],
                    EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER
                )
            );
            delete new_events[e];
        } else {
            //New event. Just throw the data we created before.
            state_ptr->events[ev_type] = new_events[e];
        }
    }
}


/**
 * @brief Unloads the states from memory.
 *
 * @param mt The type of mob.
 */
void unloadScript(MobType* mt) {
    for(size_t s = 0; s < mt->states.size(); s++) {
        MobState* s_ptr = mt->states[s];
        
        for(size_t e = 0; e < N_MOB_EVENTS; e++) {
            MobEvent* e_ptr = s_ptr->events[e];
            if(!e_ptr) continue;
            
            for(size_t a = 0; a < e_ptr->actions.size(); a++) {
                delete e_ptr->actions[a];
            }
            
            e_ptr->actions.clear();
            delete e_ptr;
            
        }
        
        delete s_ptr;
        
    }
    mt->states.clear();
}
