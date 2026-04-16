/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the script action classes and related functions.
 */


#pragma once

#include <string>
#include <vector>


#include "action_types.h"
#include "event_types.h"


using std::string;
using std::vector;


class ScriptActionType;
class ScriptDef;
class ScriptVM;


/**
 * @brief Function to run custom script actions with.
 *
 * The first parameter is the script VM running the action.
 * The second parameter depends on the function.
 * The third parameter depends on the function.
 */
typedef void (*ScriptActionCustomCode)(ScriptVM* vm, void* info1, void* info2);


/**
 * @brief Definition of a specific call to a script action
 * in the script.
 */
class ScriptActionDef {

public:

    //--- Public members ---
    
    //Action type to run, if any.
    ScriptActionType* actionType = nullptr;
    
    //Custom code to run, if any.
    ScriptActionCustomCode customCode = nullptr;
    
    //List of arguments to use.
    vector<string> args;
    
    //List of which arguments are variable names.
    vector<bool> argIsVar;
    
    //If something went wrong in parsing it, this describes the error.
    string customError;
    
    //Event the action belongs to, if any.
    FSM_EV parentEvent = FSM_EV_UNKNOWN;
    
    
    //--- Public function declarations ---
    
    explicit ScriptActionDef(SCRIPT_ACTION type = SCRIPT_ACTION_UNKNOWN);
    explicit ScriptActionDef(ScriptActionCustomCode code);
    bool loadFromDataNode(DataNode* node, ScriptDef* scriptDef);
    bool run(ScriptVM* m, void* customData1, void* customData2);
    void unload();
    
};


/**
 * @brief Definition of a block of script actions.
 */
class ScriptActionBlockDef {

public:

    //--- Public members ---
    
    //Actions to run.
    vector<ScriptActionDef*> list;


    //--- Public function declarations ---

    bool loadFromDataNode(
        DataNode* node, ScriptDef* scriptDef, Bitmask8* outFlags = nullptr
    );
    bool assertActions(DataNode* dn);
    void run(
        ScriptVM* scriptVM,
        void* customData1 = nullptr, void* customData2 = nullptr
    );
    void unload();

};


/**
 * @brief Info about how to run a specific instance of a script action.
 */
struct ScriptActionInstRunData {

    //--- Public members ---
    
    //Script VM under which the action will be run.
    ScriptVM* scriptVM = nullptr;
    
    //Action definition information.
    ScriptActionDef* actionDef = nullptr;
    
    //Arguments used.
    vector<string> args;
    
    //Event custom data 1.
    void* customData1 = nullptr;
    
    //Event custom data 2.
    void* customData2 = nullptr;
    
    //Return value, if applicable.
    bool returnValue = false;
    
    
    //--- Public function declarations ---
    
    ScriptActionInstRunData(ScriptVM* scriptVM, ScriptActionDef* call);
    
};
