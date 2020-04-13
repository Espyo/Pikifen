/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the particle class and particle-related functions.
 */

#ifndef REPLAY_INCLUDED
#define REPLAY_INCLUDED

#include <vector>

#include <allegro5/allegro.h>

#include "mobs/enemy.h"
#include "mobs/leader.h"
#include "mobs/mob.h"
#include "mobs/onion.h"
#include "mobs/pikmin.h"
#include "mobs/treasure.h"
#include "utils/geometry_utils.h"

using std::size_t;
using std::string;
using std::vector;


enum REPLAY_ELEMENT_TYPES {
    REPLAY_ELEMENT_LEADER,
    REPLAY_ELEMENT_PIKMIN,
    REPLAY_ELEMENT_ENEMY,
    REPLAY_ELEMENT_TREASURE,
    REPLAY_ELEMENT_ONION,
    REPLAY_ELEMENT_OBSTACLE,
};

enum REPLAY_EVENT_TYPES {
    //A replay element was added.
    REPLAY_EVENT_ADDED,
    //A replay element was removed.
    REPLAY_EVENT_REMOVED,
    //The player switched to a different leader.
    REPLAY_EVENT_LEADER_SWITCHED,
};


/* ----------------------------------------------------------------------------
 * Represents a Pikmin, a leader, or any other object we want to keep in
 * the replay.
 */
struct replay_element {
    //Type of element this represents.
    unsigned char type;
    //Its current position.
    point pos;
    
    replay_element(const unsigned char type, const point &pos);
};


/* ----------------------------------------------------------------------------
 * Represents some event in the playthrough that is important to save in
 * the replay, like a new element getting added, one getting removed,
 * the player switching leaders, etc.
 */
struct replay_event {
    //Type of event.
    unsigned char type;
    //Informational data about the event.
    size_t data;
    
    replay_event(
        const unsigned char type, const size_t data
    );
};


/* ----------------------------------------------------------------------------
 * Represents a point in time of the replay. This has a collection of
 * elements, as well as their state at this point in time.
 */
struct replay_state {
    //List of elements.
    vector<replay_element> elements;
    //List of events that happened here.
    vector<replay_event> events;
};


/* ----------------------------------------------------------------------------
 * A replay contains data about a playthrough of an area. It contains very
 * minimal and abstract data about what happened, such as what Pikmin
 * have moved where and when, considering the replay is only meant for the
 * player to review their strategy, not to actually watch the action again.
 * This replay is state-based, not delta-based. This means it does not save
 * the changes that have happened every moment, but rather saves the entire
 * relevant data of every moment.
 */
class replay {
public:
    vector<replay_state> states;
    
    replay();
    
    void add_state(
        const vector<leader*> &leader_list,
        const vector<pikmin*> &pikmin_list,
        const vector<enemy*> &enemy_list,
        const vector<treasure*> &treasure_list,
        const vector<onion*> &onion_list,
        const vector<mob*> &obstacle_list,
        const size_t cur_leader_nr
    );
    void clear();
    void finish_recording();
    void load_from_file(const string &file_name);
    void save_to_file(const string &file_name);
    
private:
    vector<mob*> prev_state_mobs;
    size_t prev_leader_nr;
    
};

#endif //ifndef REPLAY_INCLUDED
