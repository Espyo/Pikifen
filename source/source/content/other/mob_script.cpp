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

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/mob.h"
#include "../mob_script/gen_mob_fsm.h"
#include "../mob_type/mob_type.h"
#include "mob_script_action.h"
#include "particle.h"


/**
 * @brief Creates a new action call for the current event, one that changes
 * the mob's state to something else.
 *
 * @param newState State to change to.
 */
void EasyFsmCreator::changeState(const string& newState) {
    curEvent->actions.push_back(new MobActionCall(MOB_ACTION_SET_STATE));
    curEvent->actions.back()->args.push_back(newState);
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
void EasyFsmCreator::newState(const string& name, size_t id) {
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
void EasyFsmCreator::run(CustomActionCode code) {
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
    const DataNode* node, const vector<MobActionCall*>& actions
) :
    actions(actions) {
    
#define r(name, number) \
    else if(n == (name)) type = (number)
    
    const string& n = node->name;
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
MobEvent::MobEvent(const MOB_EV t, const vector<MobActionCall*>& a) :
    type(t),
    actions(a) {
    
}


/**
 * @brief Runs a mob event. Basically runs all actions within.
 *
 * @param m The mob.
 * @param customData1 Custom argument #1 to pass to the code.
 * @param customData2 Custom argument #2 to pass to the code.
 */
void MobEvent::run(Mob* m, void* customData1, void* customData2) {
    if(m->parent && m->parent->relayEvents) {
        m->parent->m->fsm.runEvent(type, customData1, customData2);
        if(!m->parent->handleEvents) {
            return;
        }
    }
    
    enum FLOW_CODE {
        FLOW_CODE_NONE,
        FLOW_CODE_CONDITION,
        FLOW_CODE_CONDITION_OTHER_BRANCH,
        FLOW_CODE_JUMP,
        FLOW_CODE_DO_NOTHING,
    };
    
    FLOW_CODE flowCodeToRun = FLOW_CODE_NONE;
    bool processElseIfCondition = false;
    
    for(size_t a = 0; a < actions.size(); a++) {
    
        switch(actions[a]->action->type) {
        case MOB_ACTION_IF: {
            flowCodeToRun = FLOW_CODE_CONDITION;
            break;
        } case MOB_ACTION_ELSE_IF: {
            if(processElseIfCondition) {
                flowCodeToRun = FLOW_CODE_CONDITION;
                processElseIfCondition = false;
            } else {
                flowCodeToRun = FLOW_CODE_CONDITION_OTHER_BRANCH;
            }
            break;
        } case MOB_ACTION_ELSE: {
            flowCodeToRun = FLOW_CODE_CONDITION_OTHER_BRANCH;
            break;
        } case MOB_ACTION_GOTO: {
            flowCodeToRun = FLOW_CODE_JUMP;
            break;
        } case MOB_ACTION_END_IF:
        case MOB_ACTION_LABEL: {
            flowCodeToRun = FLOW_CODE_DO_NOTHING;
            break;
        } default: {
            flowCodeToRun = FLOW_CODE_NONE;
            break;
        }
        }
        
        switch(flowCodeToRun) {
        case FLOW_CODE_CONDITION: {
            //Condition statement. Look out for its return value, and
            //change the flow accordingly.
            bool conditionValue = actions[a]->run(m, customData1, customData2);
            
            if(conditionValue) {
                //Returned true. Execution continues as normal.
            } else {
                //Returned false. Skip to the "else", "else if",
                //or "end if" actions.
                size_t nextActionIdx = actions.size();
                size_t depth = 0;
                
                for(size_t a2 = a + 1; a2 < actions.size(); a2++) {
                    MOB_ACTION a2Type = actions[a2]->action->type;
                    if(a2Type == MOB_ACTION_IF) {
                        depth++;
                    } else if(a2Type == MOB_ACTION_ELSE) {
                        if(depth == 0) {
                            nextActionIdx = a2 + 1;
                            break;
                        }
                    } else if(a2Type == MOB_ACTION_ELSE_IF) {
                        if(depth == 0) {
                            processElseIfCondition = true;
                            nextActionIdx = a2;
                            break;
                        }
                    } else if(a2Type == MOB_ACTION_END_IF) {
                        if(depth == 0) {
                            nextActionIdx = a2 + 1;
                            break;
                        } else {
                            depth--;
                        }
                    }
                }
                a = nextActionIdx - 1;
                
            }
            
            break;
            
        } case FLOW_CODE_CONDITION_OTHER_BRANCH: {
            //If we actually managed to read an "else" or "else if",
            //that means we were running through the normal execution of some
            //"then" section. Jump to the "end if".
            size_t nextActionIdx = actions.size();
            size_t depth = 0;
            
            for(size_t a2 = a + 1; a2 < actions.size(); a2++) {
                MOB_ACTION a2Type = actions[a2]->action->type;
                if(a2Type == MOB_ACTION_IF) {
                    depth++;
                } else if(a2Type == MOB_ACTION_END_IF) {
                    if(depth == 0) {
                        nextActionIdx = a2 + 1;
                        break;
                    } else {
                        depth--;
                    }
                }
            }
            a = nextActionIdx - 1;
            break;
            
        } case FLOW_CODE_JUMP: {
            //Find the label that matches.
            size_t nextActionIdx = actions.size();
            for(size_t a2 = 0; a2 < actions.size(); a2++) {
                MOB_ACTION a2Type = actions[a2]->action->type;
                if(a2Type == MOB_ACTION_LABEL) {
                    if(actions[a]->args[0] == actions[a2]->args[0]) {
                        nextActionIdx = a2 + 1;
                        break;
                    }
                }
            }
            a = nextActionIdx - 1;
            break;
            
        } case FLOW_CODE_DO_NOTHING: {
            //Nothing to do.
            break;
            
        } case FLOW_CODE_NONE: {
            //Normal action.
            actions[a]->run(m, customData1, customData2);
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
size_t MobFsm::getStateIdx(const string& name) const {
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
 * @param customData1 Custom argument #1 to pass to the code.
 * @param customData2 Custom argument #2 to pass to the code.
 */
void MobFsm::runEvent(
    const MOB_EV type, void* customData1, void* customData2
) {
    MobEvent* e = getEvent(type);
    if(e) {
        e->run(m, customData1, customData2);
    }
}


/**
 * @brief Changes the FSM to use a different state.
 *
 * @param newState The state to change to.
 * @param info1 Data to pass on to the code after the state change.
 * This data comes from the event that started all of this.
 * @param info2 Same as info1, but a second variable.
 * @return Whether it succeeded.
 */
bool MobFsm::setState(size_t newState, void* info1, void* info2) {

    //Run the code to leave the current state.
    if(curState) {
        for(unsigned char p = STATE_HISTORY_SIZE - 1; p > 0; --p) {
            prevStateNames[p] = prevStateNames[p - 1];
        }
        prevStateNames[0] = curState->name;
        runEvent(MOB_EV_ON_LEAVE, info1, info2);
    }
    
    if(newState < m->type->states.size() && newState != INVALID) {
        //Switch states.
        curState = m->type->states[newState];
        
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
MobState::MobState(const string& name) :
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
MobState::MobState(const string& name, MobEvent* evs[N_MOB_EVENTS]) :
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
MobState::MobState(const string& name, size_t id) :
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
 * @param startingState Name of the starting state for the mob.
 * @param mt Mob type these states belong to.
 * @return The index of the starting state.
 */
size_t fixStates(
    vector<MobState*>& states, const string& startingState, const MobType* mt
) {
    size_t startingStateIdx = INVALID;
    
    //Fix actions that change the state that are using a string.
    for(size_t s = 0; s < states.size(); s++) {
        MobState* state = states[s];
        if(state->name == startingState) startingStateIdx = s;
        
        for(size_t e = 0; e < N_MOB_EVENTS; e++) {
            MobEvent* ev = state->events[e];
            if(!ev) continue;
            
            for(size_t a = 0; a < ev->actions.size(); a++) {
                MobActionCall* call = ev->actions[a];
                
                if(call->action->type == MOB_ACTION_SET_STATE) {
                    string stateName = call->args[0];
                    size_t stateIdx = 0;
                    bool foundState = false;
                    
                    if(isNumber(stateName)) continue;
                    
                    for(; stateIdx < states.size(); stateIdx++) {
                        if(states[stateIdx]->name == stateName) {
                            foundState = true;
                            break;
                        }
                    }
                    
                    if(!foundState) {
                        stateIdx = INVALID;
                        game.errors.report(
                            "State \"" + state->name +
                            "\" of the mob type \"" + mt->name +
                            "\" has an action to switch to an "
                            "unknown state: \"" + stateName + "\"!",
                            nullptr
                        );
                    }
                    
                    call->args[0] = i2s(stateIdx);
                    
                }
            }
        }
    }
    return startingStateIdx;
}


/**
 * @brief Loads the states from the script and global events data nodes.
 *
 * @param mt The type of mob the states are going to.
 * @param scriptNode The data node containing the mob's script.
 * @param globalNode The data node containing global events.
 * @param outStates The loaded states are returned into this vector.
 */
void loadScript(
    MobType* mt, DataNode* scriptNode, DataNode* globalNode,
    vector<MobState*>* outStates
) {
    size_t nNewStates = scriptNode->getNrOfChildren();
    
    //Let's save the states now, so that the state switching events
    //can know what numbers the events they need correspond to.
    for(size_t s = 0; s < nNewStates; s++) {
        DataNode* stateNode = scriptNode->getChild(s);
        bool skip = false;
        for(size_t s2 = 0; s2 < outStates->size(); s2++) {
            if((*outStates)[s2]->name == stateNode->name) {
                //Already exists, probably hardcoded. Skip this.
                skip = true;
                continue;
            }
        }
        if(!skip) {
            outStates->push_back(new MobState(stateNode->name));
        }
    }
    
    for(size_t s = 0; s < outStates->size(); s++) {
        MobState* statePtr = (*outStates)[s];
        DataNode* stateNode = scriptNode->getChildByName(statePtr->name);
        loadState(mt, stateNode, globalNode, statePtr);
        statePtr->id = s;
    }
    
    fixStates(*outStates, "", mt);
}


/**
 * @brief Loads a state from the script and global events data nodes.
 *
 * @param mt The type of mob the states are going to.
 * @param stateNode The state's data node.
 * @param globalNode The data node containing global events.
 * @param statePtr Pointer to the state to load.
 */
void loadState(
    MobType* mt, DataNode* stateNode, DataNode* globalNode,
    MobState* statePtr
) {
    size_t nEvents = stateNode->getNrOfChildren();
    size_t nGlobalEvents = globalNode->getNrOfChildren();
    if(nEvents + nGlobalEvents == 0) return;
    
    //Read the events.
    vector<MobEvent*> newEvents;
    vector<Bitmask8> newEventSettings;
    
    for(size_t e = 0; e < nEvents; e++) {
        DataNode* eventNode = stateNode->getChild(e);
        vector<MobActionCall*> actions;
        Bitmask8 settings;
        
        loadActions(mt, eventNode, &actions, &settings);
        
        newEvents.push_back(new MobEvent(eventNode, actions));
        newEventSettings.push_back(settings);
        
        assertActions(actions, eventNode);
    }
    
    //Load global events.
    vector<MobEvent*> globalEvents;
    vector<Bitmask8> globalEventSettings;
    
    for(size_t e = 0; e < nGlobalEvents; e++) {
        DataNode* eventNode = globalNode->getChild(e);
        vector<MobActionCall*> actions;
        Bitmask8 settings;
        
        loadActions(mt, eventNode, &actions, &settings);
        
        globalEvents.push_back(new MobEvent(eventNode, actions));
        globalEventSettings.push_back(settings);
        
        assertActions(actions, eventNode);
    }
    
    //Insert global events into the state.
    for(size_t e = 0; e < globalEvents.size(); e++) {
        MobEvent* globalEvent = globalEvents[e];
        Bitmask8 globalSettings = globalEventSettings[e];
        
        bool merged = false;
        for(size_t ne = 0; ne < nEvents; ne++) {
            MobEvent* evPtr = newEvents[ne];
            Bitmask8 evSettings = newEventSettings[ne];
            
            if(evPtr->type != globalEvent->type) continue;
            
            insertEventActions(
                evPtr,
                globalEvent->actions,
                hasFlag(
                    globalSettings | evSettings,
                    EVENT_LOAD_FLAG_GLOBAL_ACTIONS_AFTER
                )
            );
            merged = true;
            break;
        }
        if(merged) {
            delete globalEvent;
        } else {
            newEvents.push_back(globalEvent);
            newEventSettings.push_back(globalSettings);
        }
    }
    
    //Inject a damage event.
    if(!statePtr->events[MOB_EV_HITBOX_TOUCH_N_A]) {
        vector<MobActionCall*> daActions;
        daActions.push_back(
            new MobActionCall(GenMobFsm::beAttacked)
        );
        newEvents.push_back(
            new MobEvent(MOB_EV_HITBOX_TOUCH_N_A, daActions)
        );
        newEventSettings.push_back(0);
    }
    
    //Inject a zero health event.
    if(
        stateNode->name != mt->dyingStateName &&
        !statePtr->events[MOB_EV_ZERO_HEALTH] &&
        !isInContainer(mt->statesIgnoringDeath, stateNode->name) &&
        !mt->dyingStateName.empty()
    ) {
        vector<MobActionCall*> deActions;
        deActions.push_back(new MobActionCall(GenMobFsm::goToDyingState));
        newEvents.push_back(new MobEvent(MOB_EV_ZERO_HEALTH, deActions));
        newEventSettings.push_back(0);
    }
    
    //Inject a bottomless pit event.
    if(!statePtr->events[MOB_EV_BOTTOMLESS_PIT]) {
        vector<MobActionCall*> bpActions;
        bpActions.push_back(
            new MobActionCall(GenMobFsm::fallDownPit)
        );
        newEvents.push_back(
            new MobEvent(MOB_EV_BOTTOMLESS_PIT, bpActions)
        );
        newEventSettings.push_back(0);
    }
    
    //Inject a spray touch event.
    if(
        !statePtr->events[MOB_EV_TOUCHED_SPRAY] &&
        !isInContainer(mt->statesIgnoringSpray, stateNode->name)
    ) {
        vector<MobActionCall*> sActions;
        sActions.push_back(
            new MobActionCall(GenMobFsm::touchSpray)
        );
        newEvents.push_back(
            new MobEvent(MOB_EV_TOUCHED_SPRAY, sActions)
        );
        newEventSettings.push_back(0);
    }
    
    //Inject a hazard event.
    if(
        !statePtr->events[MOB_EV_TOUCHED_HAZARD] &&
        !isInContainer(mt->statesIgnoringHazard, stateNode->name)
    ) {
        vector<MobActionCall*> hActions;
        hActions.push_back(
            new MobActionCall(GenMobFsm::touchHazard)
        );
        newEvents.push_back(
            new MobEvent(MOB_EV_TOUCHED_HAZARD, hActions)
        );
        newEventSettings.push_back(0);
    }
    
    //Connect all new events to the state.
    for(size_t e = 0; e < newEvents.size(); e++) {
        MOB_EV evType = newEvents[e]->type;
        
        if(statePtr->events[evType]) {
            insertEventActions(
                statePtr->events[evType],
                newEvents[e]->actions,
                hasFlag(
                    newEventSettings[e],
                    EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER
                )
            );
            delete newEvents[e];
        } else {
            //New event. Just throw the data we created before.
            statePtr->events[evType] = newEvents[e];
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
        MobState* sPtr = mt->states[s];
        
        for(size_t e = 0; e < N_MOB_EVENTS; e++) {
            MobEvent* ePtr = sPtr->events[e];
            if(!ePtr) continue;
            
            for(size_t a = 0; a < ePtr->actions.size(); a++) {
                delete ePtr->actions[a];
            }
            
            ePtr->actions.clear();
            delete ePtr;
            
        }
        
        delete sPtr;
        
    }
    mt->states.clear();
}
