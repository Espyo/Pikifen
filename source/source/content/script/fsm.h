/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the finite-state machine event classes and related functions.
 */


#pragma once

#include <string>
#include <vector>

#include "../../util/general_utils.h"
#include "state.h"

using std::string;
using std::vector;


namespace FSM {
const size_t STATE_HISTORY_SIZE = 3;
};


class ScriptDef;


/**
 * @brief Definition of a finite-state machine. i.e. what states,
 * events, and actions have been defined.
 */
class FsmDef {

public:

    //--- Public members ---
    
    //Script definition it belongs to.
    ScriptDef* scriptDef = nullptr;
    
    //The full list of states, with the events and actions inside.
    vector<FsmStateDef*> states;
    
    //Index of the state the FSM starts at.
    size_t firstStateIdx = INVALID;
    
    
    //--- Public function declarations ---
    
    FsmDef(ScriptDef* scriptDef);
    bool loadFromDataNode(
        DataNode* node, DataNode* globalNode, DataNode* firstStateName,
        DataNode* fileNode
    );
    bool setFirstState(const string& name);
    void unload();
    
};


/**
 * @brief Instance of a finite-state machine.
 */
class FsmInst {

public:

    //--- Public members ---
    
    //Script VM it belongs to.
    ScriptVM* script = nullptr;
    
    //Current FSM state.
    FsmStateDef* curState = nullptr;
    
    //Conversion between pre-named states and in-file states.
    vector<size_t> preNamedConversions;
    
    //Knowing the previous states' names helps with engine or content debugging.
    string prevStateNames[FSM::STATE_HISTORY_SIZE];
    
    //If this is INVALID, use the first state index defined elsewhere.
    //Otherwise, use this.
    size_t firstStateOverride = INVALID;
    
    
    //--- Public function declarations ---
    
    FsmEventDef* getEvent(const FSM_EV type) const;
    size_t getStateIdx(const string& name) const;
    string getStateHistoryStr() const;
    void init();
    void runEvent(
        const FSM_EV type,
        void* customData1 = nullptr, void* customData2 = nullptr,
        bool resetNConsecutiveActions = true
    );
    bool setState(
        size_t newState, void* info1 = nullptr, void* info2 = nullptr
    );
    
};


#pragma region Misc. classes


/**
 * @brief The easy FSM creator makes it easy to create FSMs in C++ code.

 * For FSMs created by the game maker, the state machine is simpler,
 * and written in plain text using a data file. But for the engine and
 * some preset FSMs, like the Pikmin and leader logic, there's no good way
 * to create a finite-state machine with something as simple as plain text
 * AND still give the events custom code to run.
 * The only way is to manually create a vector of states, for every
 * state, manually create the events, and for every event, manually
 * create the actions. Boring and ugly. That's why this class was born.
 * Creating a state, event, or action, are now all a single line, much like
 * they would be in a plain text file!
 */
class EasyFsmCreator {

public:

    //--- Public function declarations ---
    
    void newState(const string& name, size_t id);
    void newEvent(const FSM_EV type);
    void changeState(const string& newState);
    void run(ScriptActionCustomCode code);
    vector<FsmStateDef*> finish();
    
private:

    //--- Private members ---
    
    //List of registered states.
    vector<FsmStateDef*> states;
    
    //State currently being staged.
    FsmStateDef* curState = nullptr;
    
    //Event currently being staged.
    FsmEventDef* curEvent = nullptr;
    
    
    //--- Private function declarations ---
    
    void commitState();
    void commitEvent();
    
};


#pragma endregion
