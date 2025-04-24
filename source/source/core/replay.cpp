/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Particle class and particle-related functions.
 */

#include <algorithm>

#include "replay.h"


using std::size_t;
using std::string;
using std::vector;


/**
 * @brief Construct a new replay object.
 */
Replay::Replay() {

    clear();
}


/**
 * @brief Adds a new state to the replay, filling it with data from the supplied
 * mob vectors.
 *
 * @param leaderList List of leaders.
 * @param pikminList List of Pikmin.
 * @param enemyList List of enemies.
 * @param treasureList List of treasures.
 * @param onionList List of Onions.
 * @param obstacleList List of mobs that represent obstacles.
 * @param curLeaderIdx Index number of the current leader.
 */
void Replay::addState(
    const vector<Leader*> &leaderList,
    const vector<Pikmin*> &pikminList,
    const vector<Enemy*> &enemyList,
    const vector<Treasure*> &treasureList,
    const vector<Onion*> &onionList,
    const vector<Mob*> &obstacleList,
    size_t curLeaderIdx
) {
    states.push_back(ReplayState());
    ReplayState* newStatePtr = &(states[states.size() - 1]);
    
    vector<Mob*> newStateMobs;
    newStateMobs.insert(
        newStateMobs.end(), leaderList.begin(), leaderList.end()
    );
    newStateMobs.insert(
        newStateMobs.end(), pikminList.begin(), pikminList.end()
    );
    newStateMobs.insert(
        newStateMobs.end(), enemyList.begin(), enemyList.end()
    );
    newStateMobs.insert(
        newStateMobs.end(), treasureList.begin(), treasureList.end()
    );
    newStateMobs.insert(
        newStateMobs.end(), onionList.begin(), onionList.end()
    );
    newStateMobs.insert(
        newStateMobs.end(), obstacleList.begin(), obstacleList.end()
    );
    
    if(!prevStateMobs.empty()) {
        for(size_t pm = 0; pm < prevStateMobs.size(); pm++) {
            //Is this mob in the list of new mobs?
            auto m =
                find(
                    newStateMobs.begin(), newStateMobs.end(),
                    prevStateMobs[pm]
                );
            if(m == newStateMobs.end()) {
                //It isn't. That means it was removed.
                ReplayEvent ev(REPLAY_EVENT_REMOVED, pm);
                newStatePtr->events.push_back(ev);
            }
        }
        
        for(size_t m = 0; m < newStateMobs.size(); m++) {
            //Is this mob in the list of previous mobs?
            auto pm =
                find(
                    prevStateMobs.begin(), prevStateMobs.end(),
                    newStateMobs[m]
                );
            if(pm == prevStateMobs.end()) {
                //It isn't. That means it's new.
                ReplayEvent ev(REPLAY_EVENT_ADDED, m);
                newStatePtr->events.push_back(ev);
            }
        }
    }
    
    if(curLeaderIdx != prevLeaderIdx) {
        ReplayEvent ev(REPLAY_EVENT_LEADER_SWITCHED, curLeaderIdx);
        newStatePtr->events.push_back(ev);
        prevLeaderIdx = curLeaderIdx;
    }
    
    newStatePtr->elements.reserve(
        leaderList.size() +
        pikminList.size() +
        enemyList.size() +
        treasureList.size() +
        onionList.size() +
        obstacleList.size()
    );
    for(size_t l = 0; l < leaderList.size(); l++) {
        newStatePtr->elements.push_back(
            ReplayElement(REPLAY_ELEMENT_LEADER, leaderList[l]->pos)
        );
    }
    for(size_t p = 0; p < pikminList.size(); p++) {
        newStatePtr->elements.push_back(
            ReplayElement(REPLAY_ELEMENT_PIKMIN, pikminList[p]->pos)
        );
    }
    for(size_t e = 0; e < enemyList.size(); e++) {
        newStatePtr->elements.push_back(
            ReplayElement(REPLAY_ELEMENT_ENEMY, enemyList[e]->pos)
        );
    }
    for(size_t t = 0; t < treasureList.size(); t++) {
        newStatePtr->elements.push_back(
            ReplayElement(REPLAY_ELEMENT_TREASURE, treasureList[t]->pos)
        );
    }
    for(size_t o = 0; o < onionList.size(); o++) {
        newStatePtr->elements.push_back(
            ReplayElement(REPLAY_ELEMENT_ONION, onionList[o]->pos)
        );
    }
    for(size_t o = 0; o < obstacleList.size(); o++) {
        newStatePtr->elements.push_back(
            ReplayElement(REPLAY_ELEMENT_OBSTACLE, obstacleList[o]->pos)
        );
    }
    
    prevStateMobs = newStateMobs;
}


/**
 * @brief Clears all data about this replay.
 */
void Replay::clear() {
    states.clear();
    prevLeaderIdx = INVALID;
    prevStateMobs.clear();
}


/**
 * @brief Finishes the recording of a new replay.
 */
void Replay::finishRecording() {
    clear();
}


/**
 * @brief Loads replay data from a file in the disk.
 *
 * @param filePath Path to the file to load from.
 */
void Replay::loadFromFile(const string &filePath) {
    clear();
    ALLEGRO_FILE* file = al_fopen(filePath.c_str(), "rb");
    
    size_t nStates = al_fread32be(file);
    states.reserve(nStates);
    
    for(size_t s = 0; s < nStates; s++) {
        states.push_back(ReplayState());
        ReplayState* sPtr = &states[states.size() - 1];
        
        size_t nElements = al_fread32be(file);
        sPtr->elements.reserve(nElements);
        
        for(size_t e = 0; e < nElements; e++) {
            sPtr->elements.push_back(
                ReplayElement(
                    (REPLAY_ELEMENT) al_fgetc(file),
                    Point(al_fread32be(file), al_fread32be(file))
                )
            );
        }
        
        size_t nEvents = al_fread32be(file);
        if(nEvents > 0) {
            sPtr->events.reserve(nEvents);
            
            for(size_t e = 0; e < nEvents; e++) {
                sPtr->events.push_back(
                    ReplayEvent(
                        (REPLAY_EVENT) al_fgetc(file),
                        al_fread32be(file)
                    )
                );
            }
        }
    }
}


/**
 * @brief Saves replay data to a file in the disk.
 *
 * @param filePath Path to the file to save to.
 */
void Replay::saveToFile(const string &filePath) const {
    ALLEGRO_FILE* file = al_fopen(filePath.c_str(), "wb");
    
    al_fwrite32be(file, (int32_t) states.size());
    for(size_t s = 0; s < states.size(); s++) {
        const ReplayState* sPtr = &states[s];
        
        al_fwrite32be(file, (int32_t) sPtr->elements.size());
        for(size_t e = 0; e < sPtr->elements.size(); e++) {
            al_fputc(file, sPtr->elements[e].type);
            al_fwrite32be(file, floor(sPtr->elements[e].pos.x));
            al_fwrite32be(file, floor(sPtr->elements[e].pos.y));
        }
        
        al_fwrite32be(file, (int32_t) sPtr->events.size());
        for(size_t e = 0; e < sPtr->events.size(); e++) {
            al_fputc(file, sPtr->events[e].type);
            al_fwrite32be(file, (int32_t) sPtr->events[e].data);
        }
    }
    
    al_fclose(file);
}


/**
 * @brief Constructs a new replay element object.
 *
 * @param type Type of element.
 * @param pos Its coordinates.
 */
ReplayElement::ReplayElement(
    const REPLAY_ELEMENT type, const Point &pos
) :
    type(type),
    pos(pos) {
    
}


/**
 * @brief Constructs a new replay event object.
 *
 * @param type Type of event.
 * @param data Any numerical data this event needs.
 */
ReplayEvent::ReplayEvent(
    const REPLAY_EVENT type, size_t data
) :
    type(type),
    data(data) {
    
}
