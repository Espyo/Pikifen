/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Script event classes and related functions.
 */


#include "fsm.h"

#include "../../core/game.h"


#pragma region Easy FSM creator


/**
 * @brief Creates a new action call for the current event, one that changes
 * the FSM's state to something else.
 *
 * @param newState State to change to.
 */
void EasyFsmCreator::changeState(const string& newState) {
    curEvent->actions.list.push_back(
        new ScriptActionDef(MOB_ACTION_SET_STATE)
    );
    curEvent->actions.list.back()->args.push_back(newState);
    curEvent->actions.list.back()->argIsVar.push_back(false);
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
 * @brief Finishes any event and state under construction and returns the
 * final vector of states.
 *
 * @return The states.
 */
vector<FsmStateDef*> EasyFsmCreator::finish() {
    commitEvent();
    commitState();
    sort(
        states.begin(), states.end(),
    [] (const FsmStateDef * ms1, const FsmStateDef * ms2) -> bool {
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
void EasyFsmCreator::newEvent(const FSM_EV type) {
    commitEvent();
    curEvent = new FsmEventDef(type);
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
    curState = new FsmStateDef(name, id);
    states.push_back(curState);
}


/**
 * @brief Creates a new action for the current event, one that
 * runs some custom code.
 *
 * @param code Function with said code.
 */
void EasyFsmCreator::run(ScriptActionCustomCode code) {
    curEvent->actions.list.push_back(new ScriptActionDef(code));
}


#pragma endregion
#pragma region FSM definition


/**
 * @brief Constructs a new FSM definition object.
 *
 * @param scriptDef The script definition it belongs to.
 */
FsmDef::FsmDef(ScriptDef* scriptDef) :
    scriptDef(scriptDef) {
    
}


/**
 * @brief Compiles some things about states, if necessary.
 *
 * @param fileNode Data node of the file it was loaded from, if any.
 * @return Whether it succeeded.
 */
bool FsmDef::compileStates(DataNode* fileNode) {
    bool success = true;
    
    //Fix actions that change the state that are using a string.
    forIdx(s, states) {
        FsmStateDef* state = states[s];
        
        for(size_t e = 0; e < N_SCRIPT_EVENTS; e++) {
            FsmEventDef* ev = state->events[e];
            if(!ev) continue;
            
            forIdx(a, ev->actions.list) {
                ScriptActionDef* call = ev->actions.list[a];
                
                if(call->actionType->type == MOB_ACTION_SET_STATE) {
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
                            "\" has an action to switch to an "
                            "unknown state: \"" + stateName + "\"!",
                            fileNode
                        );
                        success = false;
                    }
                    
                    call->args[0] = i2s(stateIdx);
                    
                }
            }
        }
    }
    
    return success;
}


/**
 * @brief Loads the FSM states from the states and global events data nodes.
 *
 * @param node The data node containing the states.
 * @param globalNode The data node containing global events.
 * @param firstStateNode The data node containing the name of the first state.
 * @param fileNode The data node of the entire file.
 * @return Whether it succeeded.
 */
bool FsmDef::loadFromDataNode(
    DataNode* node, DataNode* globalNode, DataNode* firstStateNode,
    DataNode* fileNode
) {
    bool success = true;
    size_t nNewStates = node->getNrOfChildren();
    
    //Let's save the states now, so that the state switching events
    //can know what numbers the events they need correspond to.
    for(size_t s = 0; s < nNewStates; s++) {
        DataNode* stateNode = node->getChild(s);
        bool skip = false;
        forIdx(s2, states) {
            if(states[s2]->name == stateNode->name) {
                //Already exists, probably hardcoded. Skip this.
                skip = true;
                continue;
            }
        }
        if(!skip) {
            states.push_back(new FsmStateDef(stateNode->name));
        }
    }
    
    forIdx(s, states) {
        FsmStateDef* statePtr = states[s];
        DataNode* stateNode = node->getChildByName(statePtr->name);
        success &= statePtr->loadFromDataNode(stateNode, globalNode, scriptDef);
        statePtr->id = s;
    }
    
    success &= compileStates(fileNode);
    
    //First state index.
    if(!firstStateNode->value.empty()) {
        if(!setFirstState(firstStateNode->value)) {
            game.errors.report(
                "Unknown state \"" + firstStateNode->value + "\" "
                "to set as the first state!",
                firstStateNode
            );
            success = false;
        }
    }
    
    return success;
}


/**
 * @brief Sets the first state index, given its number.
 *
 * @param name The name.
 * @return Whether it succeeded.
 */
bool FsmDef::setFirstState(const string& name) {
    forIdx(s, states) {
        if(states[s]->name == name) {
            firstStateIdx = s;
            return true;
        }
    }
    return false;
}


/**
 * @brief Unloads the FSM definition and its contents from memory.
 */
void FsmDef::unload() {
    forIdx(s, states) {
        FsmStateDef* sPtr = states[s];
        sPtr->unload();
        delete sPtr;
        
    }
    states.clear();
}


#pragma endregion
#pragma region FSM instance


/**
 * @brief Returns a pointer to an event of the given type in the current state,
 * if it exists.
 *
 * @param type The event's type.
 * @return The event.
 */
FsmEventDef* FsmInst::getEvent(const FSM_EV type) const {
    return curState->events[type];
}


/**
 * @brief Returns a string containing the FSM state history.
 * This is used for debugging engine or content problems.
 *
 * @return The string.
 */
string FsmInst::getStateHistoryStr() const {
    string str = "State history: ";
    
    if(!curState) {
        str += "No current state!";
        return str;
    }
    
    str += curState->name;
    
    for(size_t s = 0; s < FSM::STATE_HISTORY_SIZE; s++) {
        str += ", " + prevStateNames[s];
    }
    str += ".";
    
    return str;
}


/**
 * @brief Returns the index of the specified state.
 *
 * @param name The state's name.
 * @return The index, or INVALID if it doesn't exist.
 */
size_t FsmInst::getStateIdx(const string& name) const {
    forIdx(s, script->scriptDef->fsm.states) {
        if(script->scriptDef->fsm.states[s]->name == name) {
            return s;
        }
    }
    return INVALID;
}


/**
 * @brief Initializes the FSM.
 */
void FsmInst::init() {
    bool firstStateSuccess =
        setState(
            firstStateOverride != INVALID ?
            firstStateOverride :
            firstStateOverride != INVALID ?
            firstStateOverride :
            script->scriptDef->fsm.firstStateIdx
        );
        
    if(!firstStateSuccess) {
        //If something went wrong, give it some dummy state.
        curState = game.dummyScriptState;
    };
}


/**
 * @brief Runs an event in the current state, if it exists.
 *
 * @param type The event's type.
 * @param customData1 Custom argument #1 to pass to the code.
 * @param customData2 Custom argument #2 to pass to the code.
 */
void FsmInst::runEvent(
    const FSM_EV type, void* customData1, void* customData2
) {
    FsmEventDef* e = getEvent(type);
    if(e) {
        e->run(script, customData1, customData2);
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
bool FsmInst::setState(size_t newState, void* info1, void* info2) {
    //Run the code to leave the current state.
    if(curState) {
        for(unsigned char p = FSM::STATE_HISTORY_SIZE - 1; p > 0; --p) {
            prevStateNames[p] = prevStateNames[p - 1];
        }
        prevStateNames[0] = curState->name;
        runEvent(FSM_EV_ON_LEAVE, info1, info2);
    }
    
    if(newState < script->scriptDef->fsm.states.size() && newState != INVALID) {
        //Switch states.
        curState = script->scriptDef->fsm.states[newState];
        
        //Run the code to enter the new state.
        runEvent(FSM_EV_ON_ENTER, info1, info2);
        
        return true;
    }
    
    return false;
    
}


#pragma endregion
