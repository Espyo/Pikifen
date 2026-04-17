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

#include "fsm.h"
#include "script_utils.h"

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
    ScriptActionBlockDef initActions;
    
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
    
    //Variables.
    map<string, string> vars;
    
    
    //--- Public function declarations ---
    
    ScriptVM();
    void init(ScriptDef* scriptDef, Mob* mobPtr = nullptr);
    void tick(float deltaT);
    void clear();
    void focusOnMob(Mob* m);
    void unfocusFromMob();
    void setTimer(float time);
    void setVar(const string& name, const string& value);
    string getMakerToolVarsStr() const;
    
};
