/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the FSM state classes and related functions.
 */


#pragma once

#include <string>
#include <vector>

#include "../../util/general_utils.h"
#include "event.h"


using std::string;
using std::vector;


/**
 * @brief An FSM state's definition. This is just the definition of the state,
 * not runtime information. As such, multiple FSMs can use the same state.
 */
class FsmStateDef {

public:

    //--- Public members ---
    
    //Name of the state.
    string name;
    
    //State ID.
    size_t id = INVALID;
    
    //List of events to handle in this state.
    FsmEventDef* events[N_FSM_EVENTS];
    
    
    //--- Public function declarations ---
    
    explicit FsmStateDef(const string& name);
    FsmStateDef(const string& name, FsmEventDef* evs[N_FSM_EVENTS]);
    FsmStateDef(const string& name, size_t id);
    void createAndAddConvenientEvents();
    void mergeEvents(
        const vector<FsmEventDef*>& newEvents,
        const vector<Bitmask8>& newEventFlags
    );
    FsmEventDef* getEvent(const FSM_EV type) const;
    bool loadFromDataNode(
        DataNode* node, DataNode* globalNode, ScriptDef* scriptDef
    );
    void unload();
    
};
