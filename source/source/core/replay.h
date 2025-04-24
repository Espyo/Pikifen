/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the particle class and particle-related functions.
 */

#pragma once

#include <vector>

#include <allegro5/allegro.h>

#include "../content/mob/enemy.h"
#include "../content/mob/leader.h"
#include "../content/mob/mob.h"
#include "../content/mob/onion.h"
#include "../content/mob/pikmin.h"
#include "../content/mob/treasure.h"
#include "../util/general_utils.h"
#include "../util/geometry_utils.h"


using std::size_t;
using std::string;
using std::vector;


//Types of elements in a replay.
enum REPLAY_ELEMENT {

    //A leader.
    REPLAY_ELEMENT_LEADER,
    
    //A Pikmin.
    REPLAY_ELEMENT_PIKMIN,
    
    //An enemy.
    REPLAY_ELEMENT_ENEMY,
    
    //A treasure.
    REPLAY_ELEMENT_TREASURE,
    
    //An Onion.
    REPLAY_ELEMENT_ONION,
    
    //An obstacle.
    REPLAY_ELEMENT_OBSTACLE,
    
};


//Types of events that can happen in a replay.
enum REPLAY_EVENT {

    //A replay element was added.
    REPLAY_EVENT_ADDED,
    
    //A replay element was removed.
    REPLAY_EVENT_REMOVED,
    
    //The player switched to a different leader.
    REPLAY_EVENT_LEADER_SWITCHED,
    
};


/**
 * @brief Represents a Pikmin, a leader, or any other object we want to keep in
 * the replay.
 */
struct ReplayElement {

    //--- Members ---
    
    //Type of element this represents.
    REPLAY_ELEMENT type = REPLAY_ELEMENT_LEADER;
    
    //Its current position.
    Point pos;
    
    
    //--- Function declarations ---
    
    ReplayElement(const REPLAY_ELEMENT type, const Point &pos);
    
};


/**
 * @brief Represents some event in the playthrough that is important to save in
 * the replay, like a new element getting added, one getting removed,
 * the player switching leaders, etc.
 */
struct ReplayEvent {

    //--- Members ---
    
    //Type of event.
    REPLAY_EVENT type = REPLAY_EVENT_ADDED;
    
    //Informational data about the event.
    size_t data = 0;
    
    
    //--- Function declarations ---
    
    ReplayEvent(
        const REPLAY_EVENT type, size_t data
    );
    
};


/**
 * @brief Represents a point in time of the replay. This has a collection of
 * elements, as well as their state at this point in time.
 */
struct ReplayState {

    //--- Members ---
    
    //List of elements.
    vector<ReplayElement> elements;
    
    //List of events that happened here.
    vector<ReplayEvent> events;
    
};


/**
 * @brief A replay contains data about a playthrough of an area.
 *
 * It contains very
 * minimal and abstract data about what happened, such as what Pikmin
 * have moved where and when, considering the replay is only meant for the
 * player to review their strategy, not to actually watch the action again.
 * This replay is state-based, not delta-based. This means it does not save
 * the changes that have happened every moment, but rather saves the entire
 * relevant data of every moment.
 */
class Replay {

public:

    //--- Members ---
    
    //States.
    vector<ReplayState> states;
    
    
    //--- Function declarations ---
    
    Replay();
    void addState(
        const vector<Leader*> &leaderList,
        const vector<Pikmin*> &pikminList,
        const vector<Enemy*> &enemyList,
        const vector<Treasure*> &treasureList,
        const vector<Onion*> &onionList,
        const vector<Mob*> &obstacleList,
        size_t curLeaderIdx
    );
    void clear();
    void finishRecording();
    void loadFromFile(const string &filePath);
    void saveToFile(const string &filePath) const;
    
private:

    //--- Members ---
    
    //List of mobs in the previous state.
    vector<Mob*> prevStateMobs;
    
    //Index of the previous leader.
    size_t prevLeaderIdx = INVALID;
    
};
