/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the script classes and related functions.
 */


#pragma once

#include <map>
#include <string>
#include <vector>

#include "fsm.hpp"
#include "script_utils.hpp"
#include "../../core/misc_structs.hpp"

using std::map;
using std::string;
using std::vector;


class Area;
class MobType;


/**
 * @brief Contains a definition about how a script should run. It also contains
 * a definition of an FSM.
 */
class ScriptDef {

public:

    //--- Public members ---
    
    //Mob type it belongs to, if any.
    MobType* mobType = nullptr;
    
    //Area it belongs to, if any.
    Area* area = nullptr;
    
    //Actions to run on script startup.
    ScriptActionListDef initActions;
    
    //Actions to run when the area or mob is fully loaded and ready.
    ScriptActionListDef readyActions;
    
    //Definition of the finite-state machine.
    FsmDef fsm;
    
    
    //--- Public function declarations ---
    
    ScriptDef();
    bool loadFromDataNode(DataNode* node);
    SCRIPT_CONTEXT getContext() const;
    string getContextName() const;
    bool checkContextFlags(Bitmask8 flags) const;
    void unload();
    
};


class Mob;
class Area;


/**
 * @brief A script's virtual machine. It contains an instance of a
 * script, relevant data, and an instance of a finite-state machine.
 */
class ScriptVM {

public:

    //--- Public members ---
    
    //Script definition that defines it.
    ScriptDef* scriptDef = nullptr;
    
    //Mob that this VM belongs to, if any.
    Mob* mob = nullptr;
    
    //The finite-state machine.
    FsmInst fsm;
    
    //Custom timer.
    Timer timer;
    
    //The mob it has focus on.
    Mob* focusedMob = nullptr;
    
    //The mob that the next actions will be run as. nullptr means the
    //current mob, if applicable.
    Mob* nextActionSurrogateMob = nullptr;
    
    //Script variables.
    ScriptVarManager vars;
    
    
    //--- Public function declarations ---
    
    ScriptVM();
    void init(ScriptDef* scriptDef, Mob* mobPtr = nullptr);
    void initFsm();
    void tick(float deltaT);
    void clear();
    void runReadyActions();
    void focusOnMob(Mob* m);
    void unfocusFromMob();
    void setTimer(float time);
    Mob* getRunnerMob() const;
    ScriptVM* getRunnerScriptVM();
    string getMakerToolVarsStr() const;
    
};


/**
 * @brief Auxiliary data for the execution of a script. To save on memory,
 * this contains data that can and should be instantiated once, and then
 * used globally.
 */
class ScriptExecutionAuxData {

public:
    
    //--- Public members ---
    
    //Consecutive count of actions ran in a row. Used to detect infinite loops.
    size_t nConsecutiveActions = 0;
    
    //When entering a for loop, are we meant to increment the iterator
    //variable, or initialize it?
    bool forLoopEntryNeedsIncrement = false;
    
    
    //--- Public function declarations ---
    
    void reset();
    
};
