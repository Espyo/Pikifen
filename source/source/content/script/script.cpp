/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scripting classes and related functions.
 */

#include <algorithm>

#include "script.h"


#pragma region Script definition


/**
 * @brief Constructs a new script definition object.
 */
ScriptDef::ScriptDef() :
    fsm(this) {
    
}


/**
 * @brief Loads a script definition from a data node.
 *
 * @param node Node to load from.
 * @return Whether it succeeded.
 */
bool ScriptDef::loadFromDataNode(DataNode* node) {
    bool success = true;
    
    //Init actions.
    success &=
        initActions.loadFromDataNode(
            node->getChildByName("init"),
            this
        );
        
    //The FSM.
    success &=
        fsm.loadFromDataNode(
            node->getChildByName("script"), node->getChildByName("global"),
            node->getChildByName("first_state"), node
        );
        
    return success;
}


/**
 * @brief Unloads the script definition and its contents from memory.
 */
void ScriptDef::unload() {
    initActions.unload();
    fsm.unload();
}


#pragma endregion
#pragma region Script VM


/**
 * @brief Constructs a new script VM object.
 */
ScriptVM::ScriptVM() {
    fsm.script = this;
}


/**
 * @brief Initializes the script VM, setting up its data, running its
 * init actions, etc.
 *
 * @param scriptDef Script definition it should follow.
 * @param mobPtr Whether there is an associated mob.
 */
void ScriptVM::init(ScriptDef* scriptDef, Mob* mobPtr) {
    this->scriptDef = scriptDef;
    mob = mobPtr;
    timer.stop();
    vars.clear();
    
    scriptDef->initActions.run(this);
    fsm.init();
}


/**
 * @brief Returns a string representing the values of all script vars,
 * formatted in a way that's friendly for the info maker tool.
 *
 * @return The string.
 */
string ScriptVM::getMakerToolVarsStr() const {
    string result = "Vars: ";
    
    if(vars.empty()) {
        result += "(None)";
        return result;
    }
    
    for(const auto& v : vars) {
        result += v.first + "=" + v.second + "; ";
    }
    result.erase(result.size() - 2, 2);
    result = wordWrap(result, 98, 2);
    
    return result;
}


/**
 * @brief Ticks time by one frame of logic. This processes the timer and
 * runs some convenient events.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void ScriptVM::tick(float deltaT) {
    //Timer event.
    FsmEventDef* timerEv = fsm.getEvent(MOB_EV_TIMER);
    if(timer.duration > 0) {
        if(timer.timeLeft > 0) {
            timer.tick(deltaT);
            if(timer.timeLeft == 0.0f && timerEv) {
                timerEv->run(this);
            }
        }
    }
    
    //Tick event.
    fsm.runEvent(FSM_EV_ON_TICK);
}


#pragma endregion
