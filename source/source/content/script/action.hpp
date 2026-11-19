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


#include "action_types.hpp"
#include "event_types.hpp"


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
    
    //Event the action belongs to, if any.
    FSM_EV parentEvent = FSM_EV_UNKNOWN;
    
    //Line in the data file from which it was read. If 0, this means
    //this action definition was created or loaded from something that isn't
    //the data node relevant to the mob type or area it is used in.
    size_t dataFileLine = 0;
    
    
    //--- Public function declarations ---
    
    explicit ScriptActionDef(SCRIPT_ACTION type = SCRIPT_ACTION_UNKNOWN);
    explicit ScriptActionDef(ScriptActionCustomCode code);
    bool loadFromDataNode(
        DataNode* node, ScriptDef* scriptDef,
        const vector<ScriptActionDef*>& actionList,
        bool isDataNodeRelevant = true
    );
    bool run(ScriptVM* m, void* customData1, void* customData2);
    void unload();
    
};


/**
 * @brief Definition of a list of script actions.
 */
class ScriptActionListDef {

public:

    //--- Public misc. declarations ---
    
    //Ways to process the current action, mostly in regards to the list's flow.
    enum PROCESS_TYPE {
    
        //Run the action. Then proceed to the next sequential action.
        PROCESS_TYPE_NORMAL,
        
        //Run the action and check the condition. Then either proceed to
        //the next sequential action or jump to another action accordingly.
        PROCESS_TYPE_CHECK_CONDITION,
        
        //Do not run this action. Then jump to the end of the current construct.
        PROCESS_TYPE_JUMP_TO_CONSTRUCT_END,
        
        //Do not run this action. Then jump to the appropriate label action.
        PROCESS_TYPE_JUMP_ELSEWHERE,
        
        //Do not run this action. Then proceed to the next sequential action.
        PROCESS_TYPE_DO_NOTHING,
        
    };
    
    
    //--- Public members ---
    
    //Actions to run.
    vector<ScriptActionDef*> list;
    
    
    //--- Public function declarations ---
    
    bool loadFromDataNode(
        DataNode* node, ScriptDef* scriptDef, Bitmask8* outFlags = nullptr
    );
    bool compile(DataNode* dn);
    void run(
        ScriptVM* scriptVM,
        void* customData1 = nullptr, void* customData2 = nullptr
    );
    void unload();
    
    
private:

    //--- Private members ---
    
    //Depth of each action. Cache for convenience.
    vector<size_t> depthsCache;
    
    
    //--- Private function declarations ---
    
    PROCESS_TYPE getActionProcessType(
        size_t actionIdx, bool* mustProcessElseIfConditionPtr
    ) const;
    size_t processAction(
        size_t actionIdx, ScriptActionListDef::PROCESS_TYPE processType,
        bool* mustProcessElseIfConditionPtr, ScriptVM* scriptVM,
        void* customData1 = nullptr, void* customData2 = nullptr
    );
    size_t processActionCheckCondition(
        size_t actionIdx, bool* mustProcessElseIfConditionPtr,
        ScriptVM* scriptVM,
        void* customData1 = nullptr, void* customData2 = nullptr
    );
    size_t processActionJumpToConstructEnd(
        size_t actionIdx, ScriptVM* scriptVM,
        void* customData1 = nullptr, void* customData2 = nullptr
    );
    size_t processActionJumpToLabel(
        size_t actionIdx, ScriptVM* scriptVM,
        void* customData1 = nullptr, void* customData2 = nullptr
    );
    
};


/**
 * @brief Info about how to run a specific instance of a script action.
 */
class ScriptActionInstRunData {

public:

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
