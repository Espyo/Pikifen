/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Script event classes and related functions.
 */


#include "state.h"

#include "../../core/game.h"


#pragma region FSM state definition


/**
 * @brief Constructs a new FSM state object.
 *
 * @param name The state's name.
 */
FsmStateDef::FsmStateDef(const string& name) :
    name(name) {
    
    for(size_t e = 0; e < N_SCRIPT_EVENTS; e++) {
        events[e] = nullptr;
    }
}


/**
 * @brief Constructs a new FSM state object.
 *
 * @param name The state's name.
 * @param evs Its events.
 */
FsmStateDef::FsmStateDef(
    const string& name, FsmEventDef* evs[N_SCRIPT_EVENTS]
) :
    name(name) {
    
    for(size_t e = 0; e < N_SCRIPT_EVENTS; e++) {
        events[e] = evs[e];
    }
}


/**
 * @brief Constructs a new FSM state object.
 *
 * @param name The state's name.
 * @param id Its ID, for sorting on the vector of states.
 */
FsmStateDef::FsmStateDef(const string& name, size_t id) :
    name(name),
    id(id) {
    
    for(size_t e = 0; e < N_SCRIPT_EVENTS; e++) {
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
FsmEventDef* FsmStateDef::getEvent(const FSM_EV type) const {
    return events[type];
}


/**
 * @brief Loads a state from the script and global events data nodes.
 *
 * @param node The state's data node.
 * @param globalNode The data node containing global events.
 * @param scriptDef Script definition it belongs to, if any.
 * @return Whether it succeeded.
 */
bool FsmStateDef::loadFromDataNode(
    DataNode* node, DataNode* globalNode, ScriptDef* scriptDef
) {
    bool success = true;
    size_t nEvents = node->getNrOfChildren();
    size_t nGlobalEvents = globalNode->getNrOfChildren();
    if(nEvents + nGlobalEvents == 0) return true;
    
    //Read the events.
    vector<FsmEventDef*> newEvents;
    vector<Bitmask8> newEventFlags;
    
    for(size_t e = 0; e < nEvents; e++) {
        DataNode* eventNode = node->getChild(e);
        vector<ScriptActionDef*> actions;
        Bitmask8 flags;
        FsmEventDef* newEvent = new FsmEventDef();
        success &= newEvent->loadFromDataNode(eventNode, scriptDef, &flags);
        newEvents.push_back(newEvent);
        newEventFlags.push_back(flags);
        newEvent->actions.assertActions(eventNode);
    }
    
    //Load global events.
    vector<FsmEventDef*> globalEvents;
    vector<Bitmask8> globalEventFlags;
    
    for(size_t e = 0; e < nGlobalEvents; e++) {
        DataNode* eventNode = globalNode->getChild(e);
        vector<ScriptActionDef*> actions;
        Bitmask8 flags;
        FsmEventDef* newEvent = new FsmEventDef();
        success &= newEvent->loadFromDataNode(eventNode, scriptDef, &flags);
        globalEvents.push_back(newEvent);
        globalEventFlags.push_back(flags);
        newEvent->actions.assertActions(eventNode);
    }
    
    //Merge global event actions with the dedicated event actions.
    for(size_t e = 0; e < globalEvents.size(); e++) {
        FsmEventDef* globalEvent = globalEvents[e];
        Bitmask8 globalFlags = globalEventFlags[e];
        
        bool merged = false;
        for(size_t ne = 0; ne < nEvents; ne++) {
            FsmEventDef* evPtr = newEvents[ne];
            Bitmask8 evFlags = newEventFlags[ne];
            
            if(evPtr->type != globalEvent->type) continue;
            evPtr->insertActions(
                globalEvent->actions.list,
                hasFlag(
                    globalFlags | evFlags,
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
            newEventFlags.push_back(globalFlags);
        }
    }
    
    //Add these events to the state.
    mergeEvents(newEvents, newEventFlags);
    
    //Let the mob type do extra processing, if necessary.
    if(scriptDef->mobType) {
        scriptDef->mobType->handleLoadedScriptState(this);
    }
    
    return success;
}


/**
 * @brief Merges a list of new events into the state.
 *
 * @param newEvents The new events.
 * @param newEventFlags The new events' flags.
 */
void FsmStateDef::mergeEvents(
    const vector<FsmEventDef*>& newEvents,
    const vector<Bitmask8>& newEventFlags
) {
    for(size_t e = 0; e < newEvents.size(); e++) {
        FSM_EV evType = newEvents[e]->type;
        
        if(events[evType]) {
            //Existing event. Insert the new actions.
            events[evType]->insertActions(
                newEvents[e]->actions.list,
                hasFlag(
                    newEventFlags[e],
                    EVENT_LOAD_FLAG_CUSTOM_ACTIONS_AFTER
                )
            );
            delete newEvents[e];
        } else {
            //New event. Just throw the data we created before.
            events[evType] = newEvents[e];
        }
    }
}


/**
 * @brief Unloads the state definition and its contents from memory.
 */
void FsmStateDef::unload() {
    for(size_t e = 0; e < N_SCRIPT_EVENTS; e++) {
        FsmEventDef* ePtr = events[e];
        if(!ePtr) continue;
        ePtr->unload();
        delete ePtr;
    }
}


#pragma endregion
