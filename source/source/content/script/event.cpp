/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * FSM event classes and related functions.
 */


#include "event.h"

#include "../../core/game.h"


#pragma region FSM event definition


/**
 * @brief Constructs a new FSM event definition object.
 *
 * @param type Its type.
 * @param actions List of its actions.
 */
FsmEventDef::FsmEventDef(FSM_EV type, const vector<ScriptActionDef*>& actions) :
    type(type) {
    
    this->actions.list = actions;
}


/**
 * @brief Add a vector of actions onto the event.
 *
 * @param newActions Vector of actions to insert.
 * @param atEnd Are the actions inserted at the end?
 */
void FsmEventDef::insertActions(
    const vector<ScriptActionDef*>& newActions, bool atEnd
) {
    auto it = atEnd ? actions.list.end() : actions.list.begin();
    actions.list.insert(it, newActions.begin(), newActions.end());
}


/**
 * @brief Loads the event and its actions from a data node.
 *
 * @param node The data node.
 * @param scriptDef Script definition it belongs to, if any.
 * @param outActionFlags If not nullptr, the flags for the list of actions
 * is returned here.
 */
bool FsmEventDef::loadFromDataNode(
    DataNode* node, ScriptDef* scriptDef, Bitmask8* outActionFlags
) {
    bool success = true;
    
    bool typeFound;
    type = enumGetValue(scriptEvScriptFileINames, node->name, &typeFound);
    
    if(!typeFound) {
        type = FSM_EV_UNKNOWN;
        game.errors.report(
            "Unknown script event name \"" + node->name + "\"!", node
        );
        success = false;
    }
    
    success &= actions.loadFromDataNode(node, scriptDef, outActionFlags);
    
    forIdx(a, actions.list) {
        actions.list[a]->parentEvent = type;
    }
    
    return success;
}


/**
 * @brief Runs the block of actions in the event.
 *
 * @param scriptVM Script VM in which these actions will be run.
 * @param customData1 Custom argument #1.
 * @param customData2 Custom argument #2.
 */
void FsmEventDef::run(
    ScriptVM* scriptVM, void* customData1, void* customData2
) {
    if(scriptVM->mob) {
        if(scriptVM->mob->parent && scriptVM->mob->parent->relayEvents) {
            Mob* parentMob = scriptVM->mob->parent->m;
            parentMob->scriptVM.fsm.runEvent(type, customData1, customData2);
            if(scriptVM->mob->parent->handleEvents) {
                return;
            }
        }
    }
    
    actions.run(scriptVM, customData1, customData2);
}


/**
 * @brief Unloads an event definition and its contents from memory.
 */
void FsmEventDef::unload() {
    actions.unload();
}


#pragma endregion
