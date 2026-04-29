/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the FSM event classes and related functions.
 */


#pragma once

#include <string>
#include <vector>

#include "action.h"
#include "event_types.h"


using std::string;
using std::vector;


/**
 * @brief An FSM event's definition, and the list of actions to run
 * when it triggers. This is just the definition of the event,
 * not runtime information. As such, multiple FSMs can use the same event.
 */
class FsmEventDef {

public:

    //--- Public members ---
    
    //Type of event.
    FSM_EV type = FSM_EV_UNKNOWN;

    //Block of actions to run.
    ScriptActionBlockDef actions;


    //--- Public function declarations ---

    FsmEventDef(
        FSM_EV type = FSM_EV_UNKNOWN,
        const vector<ScriptActionDef*>& actions = vector<ScriptActionDef*>()
    );
    bool loadFromDataNode(
        DataNode* node, ScriptDef* scriptDef, Bitmask8* outActionFlags = nullptr
    );
    void insertActions(
        const vector<ScriptActionDef*>& newActions, bool atEnd
    );
    void run(
        ScriptVM* scriptVM,
        void* customData1 = nullptr, void* customData2 = nullptr,
        bool resetNConsecutiveActions = true
    );
    void unload();
    
};
