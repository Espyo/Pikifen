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


/* ----------------------------------------------------------------------------
 * Creates a new replay.
 */
replay::replay() :
    prev_leader_nr(INVALID) {
    
    clear();
}


/* ----------------------------------------------------------------------------
 * Adds a new state to the replay, filling it with data from the supplied
 * mob vectors.
 * leader_list:
 *   List of leaders.
 * pikmin_list:
 *   List of Pikmin.
 * enemy_list:
 *   List of enemies.
 * treasure_list:
 *   List of treasures.
 * onion_list:
 *   List of Onions.
 * obstacle_list:
 *   List of mobs that represent obstacles.
 * cur_leader_nr:
 *   Index number of the current leader.
 */
void replay::add_state(
    const vector<leader*> &leader_list,
    const vector<pikmin*> &pikmin_list,
    const vector<enemy*> &enemy_list,
    const vector<treasure*> &treasure_list,
    const vector<onion*> &onion_list,
    const vector<mob*> &obstacle_list,
    const size_t cur_leader_nr
) {
    states.push_back(replay_state());
    replay_state* new_state_ptr = &(states[states.size() - 1]);
    
    vector<mob*> new_state_mobs;
    new_state_mobs.insert(
        new_state_mobs.end(), leader_list.begin(), leader_list.end()
    );
    new_state_mobs.insert(
        new_state_mobs.end(), pikmin_list.begin(), pikmin_list.end()
    );
    new_state_mobs.insert(
        new_state_mobs.end(), enemy_list.begin(), enemy_list.end()
    );
    new_state_mobs.insert(
        new_state_mobs.end(), treasure_list.begin(), treasure_list.end()
    );
    new_state_mobs.insert(
        new_state_mobs.end(), onion_list.begin(), onion_list.end()
    );
    new_state_mobs.insert(
        new_state_mobs.end(), obstacle_list.begin(), obstacle_list.end()
    );
    
    if(!prev_state_mobs.empty()) {
        for(size_t pm = 0; pm < prev_state_mobs.size(); ++pm) {
            //Is this mob in the list of new mobs?
            auto m =
                find(
                    new_state_mobs.begin(), new_state_mobs.end(),
                    prev_state_mobs[pm]
                );
            if(m == new_state_mobs.end()) {
                //It isn't. That means it was removed.
                replay_event ev(REPLAY_EVENT_REMOVED, pm);
                new_state_ptr->events.push_back(ev);
            }
        }
        
        for(size_t m = 0; m < new_state_mobs.size(); ++m) {
            //Is this mob in the list of previous mobs?
            auto pm =
                find(
                    prev_state_mobs.begin(), prev_state_mobs.end(),
                    new_state_mobs[m]
                );
            if(pm == prev_state_mobs.end()) {
                //It isn't. That means it's new.
                replay_event ev(REPLAY_EVENT_ADDED, m);
                new_state_ptr->events.push_back(ev);
            }
        }
    }
    
    if(cur_leader_nr != prev_leader_nr) {
        replay_event ev(REPLAY_EVENT_LEADER_SWITCHED, cur_leader_nr);
        new_state_ptr->events.push_back(ev);
        prev_leader_nr = cur_leader_nr;
    }
    
    new_state_ptr->elements.reserve(
        leader_list.size() +
        pikmin_list.size() +
        enemy_list.size() +
        treasure_list.size() +
        onion_list.size() +
        obstacle_list.size()
    );
    for(size_t l = 0; l < leader_list.size(); ++l) {
        new_state_ptr->elements.push_back(
            replay_element(REPLAY_ELEMENT_LEADER, leader_list[l]->pos)
        );
    }
    for(size_t p = 0; p < pikmin_list.size(); ++p) {
        new_state_ptr->elements.push_back(
            replay_element(REPLAY_ELEMENT_PIKMIN, pikmin_list[p]->pos)
        );
    }
    for(size_t e = 0; e < enemy_list.size(); ++e) {
        new_state_ptr->elements.push_back(
            replay_element(REPLAY_ELEMENT_ENEMY, enemy_list[e]->pos)
        );
    }
    for(size_t t = 0; t < treasure_list.size(); ++t) {
        new_state_ptr->elements.push_back(
            replay_element(REPLAY_ELEMENT_TREASURE, treasure_list[t]->pos)
        );
    }
    for(size_t o = 0; o < onion_list.size(); ++o) {
        new_state_ptr->elements.push_back(
            replay_element(REPLAY_ELEMENT_ONION, onion_list[o]->pos)
        );
    }
    for(size_t o = 0; o < obstacle_list.size(); ++o) {
        new_state_ptr->elements.push_back(
            replay_element(REPLAY_ELEMENT_OBSTACLE, obstacle_list[o]->pos)
        );
    }
    
    prev_state_mobs = new_state_mobs;
}


/* ----------------------------------------------------------------------------
 * Clears all data about this replay.
 */
void replay::clear() {
    states.clear();
    prev_leader_nr = INVALID;
    prev_state_mobs.clear();
}


/* ----------------------------------------------------------------------------
 * Finishes the recording of a new replay.
 */
void replay::finish_recording() {
    clear();
}


/* ----------------------------------------------------------------------------
 * Loads replay data from a file in the disk.
 * file_name:
 *   Name of the file to load from.
 */
void replay::load_from_file(const string &file_name) {
    clear();
    ALLEGRO_FILE* file = al_fopen(file_name.c_str(), "rb");
    
    size_t n_states = al_fread32be(file);
    states.reserve(n_states);
    
    for(size_t s = 0; s < n_states; ++s) {
        states.push_back(replay_state());
        replay_state* s_ptr = &states[states.size() - 1];
        
        size_t n_elements = al_fread32be(file);
        s_ptr->elements.reserve(n_elements);
        
        for(size_t e = 0; e < n_elements; ++e) {
            s_ptr->elements.push_back(
                replay_element(
                    al_fgetc(file),
                    point(al_fread32be(file), al_fread32be(file))
                )
            );
        }
        
        size_t n_events = al_fread32be(file);
        if(n_events > 0) {
            s_ptr->events.reserve(n_events);
            
            for(size_t e = 0; e < n_events; ++e) {
                s_ptr->events.push_back(
                    replay_event(al_fgetc(file), al_fread32be(file))
                );
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Saves replay data to a file in the disk.
 * file_name:
 *   Name of the file to save to.
 */
void replay::save_to_file(const string &file_name) const {
    ALLEGRO_FILE* file = al_fopen(file_name.c_str(), "wb");
    
    al_fwrite32be(file, states.size());
    for(size_t s = 0; s < states.size(); ++s) {
        const replay_state* s_ptr = &states[s];
        
        al_fwrite32be(file, s_ptr->elements.size());
        for(size_t e = 0; e < s_ptr->elements.size(); ++e) {
            al_fputc(file, s_ptr->elements[e].type);
            al_fwrite32be(file, floor(s_ptr->elements[e].pos.x));
            al_fwrite32be(file, floor(s_ptr->elements[e].pos.y));
        }
        
        al_fwrite32be(file, s_ptr->events.size());
        for(size_t e = 0; e < s_ptr->events.size(); ++e) {
            al_fputc(file, s_ptr->events[e].type);
            al_fwrite32be(file, s_ptr->events[e].data);
        }
    }
    
    al_fclose(file);
}


/* ----------------------------------------------------------------------------
 * Creates a new replay element.
 * type:
 *   Type of element. Use REPLAY_ELEMENT_*.
 * pos:
 *   Its coordinates.
 */
replay_element::replay_element(const unsigned char type, const point &pos) :
    type(type),
    pos(pos) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new replay event.
 * type:
 *   Type of event. Use REPLAY_EVENT_*.
 * data:
 *   Any numerical data this event needs.
 */
replay_event::replay_event(
    const unsigned char type, const size_t data
) :
    type(type),
    data(data) {
    
}
