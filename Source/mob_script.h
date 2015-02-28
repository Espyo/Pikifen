/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the mob script classes and
 * related functions.
 */

#ifndef MOB_EVENT_INCLUDED
#define MOB_EVENT_INCLUDED

#include <vector>

#include "data_file.h"

using namespace std;

class mob;
class mob_type;
class mob_state;

class mob_action {
public:
    unsigned short type;
    unsigned char sub_type;
    bool valid;
    vector<int> vi;
    vector<float> vf;
    vector<string> vs;
    
    void run(mob* m, size_t* action_nr);
    mob_action(data_node* dn, vector<mob_state*>* states, mob_type* mt);
};


class mob_event {
public:
    unsigned short type;
    vector<mob_action*> actions;
    
    void run(mob* m);
    mob_event(data_node* d, vector<mob_action*> a);
    mob_event(const unsigned short t, vector<mob_action*> a);
};


class mob_state {
public:
    string name;
    unsigned short id;
    vector<mob_event*> events;
    mob_event* get_event(const unsigned short type);
    
    mob_state(const string name, vector<mob_event*> e = vector<mob_event*>());
};


class mob_fsm {
public:
    mob* m;
    mob_state* cur_state;
    vector<size_t> pre_named_conversions; // Conversion between pre-named states and in-file states.
    
    void set_state(const size_t new_state);
    mob_event* get_event(const unsigned short type);
    void run_event(const unsigned short type, mob* m);
    mob_fsm(mob* m = NULL);
};


vector<mob_state*> load_script(mob_type* mt, data_node* node);


enum MOB_ACTION_TYPES {
    MOB_ACTION_UNKNOWN,
    MOB_ACTION_CHOMP_HITBOXES,
    MOB_ACTION_EAT,
    MOB_ACTION_IF,
    MOB_ACTION_MOVE,
    MOB_ACTION_SET_SPEED,
    MOB_ACTION_SET_GRAVITY,
    MOB_ACTION_PLAY_SOUND,
    MOB_ACTION_SET_VAR,
    MOB_ACTION_SET_ANIMATION,
    MOB_ACTION_SPECIAL_FUNCTION,
    MOB_ACTION_SPAWN_PROJECTILE,
    MOB_ACTION_SPAWN_PARTICLE,
    MOB_ACTION_SET_TIMER,
    MOB_ACTION_SET_HEALTH,
    MOB_ACTION_SET_STATE,
    MOB_ACTION_TURN,
    MOB_ACTION_WAIT,
};

enum MOB_EVENT_TYPES {
    MOB_EVENT_UNKNOWN,
    MOB_EVENT_ON_ENTER,
    MOB_EVENT_ON_LEAVE,
    MOB_EVENT_ANIMATION_END,
    MOB_EVENT_ATTACK_HIT,
    MOB_EVENT_ATTACK_MISS,
    MOB_EVENT_BIG_DAMAGE,
    MOB_EVENT_DAMAGE,
    MOB_EVENT_DEATH,
    MOB_EVENT_ENTER_HAZARD,
    MOB_EVENT_IDLE, //TODO is this used?
    MOB_EVENT_LEAVE_HAZARD,
    MOB_EVENT_LOSE_OBJECT,
    MOB_EVENT_LOSE_OPPONENT,
    MOB_EVENT_NEAR_OBJECT,
    MOB_EVENT_NEAR_OPPONENT,
    MOB_EVENT_PIKMIN_LAND,
    MOB_EVENT_PIKMIN_LATCH,
    MOB_EVENT_PIKMIN_TOUCH,
    MOB_EVENT_REACH_HOME,
    MOB_EVENT_REVIVAL,
    MOB_EVENT_SEE_OBJECT,
    MOB_EVENT_SEE_OPPONENT,
    MOB_EVENT_TIMER,
    MOB_EVENT_WALL,
    N_MOB_EVENTS,
};

enum MOB_ACTION_EAT_TYPES {
    MOB_ACTION_EAT_ALL,
    MOB_ACTION_EAT_NUMBER,
};

enum MOB_ACTION_MOVE_TYPES {
    MOB_ACTION_MOVE_OPPONENT,
    MOB_ACTION_MOVE_HOME,
    MOB_ACTION_MOVE_STOP,
    MOB_ACTION_MOVE_COORDS,
    MOB_ACTION_MOVE_REL_COORDS,
};

enum MOB_ACTION_SET_HEALTH_TYPES {
    MOB_ACTION_SET_HEALTH_ABSOLUTE,
    MOB_ACTION_SET_HEALTH_RELATIVE,
};

enum MOB_ACTION_SPECIAL_FUNCTION_TYPES {
    MOB_ACTION_SPECIAL_FUNCTION_DIE_START,
    MOB_ACTION_SPECIAL_FUNCTION_DIE_END,
    MOB_ACTION_SPECIAL_FUNCTION_LOOP,
};

enum MOB_ACTION_WAIT_TYPES {
    MOB_ACTION_WAIT_ANIMATION,
    MOB_ACTION_WAIT_TIME,
};


#endif // ifndef MOB_EVENT_INCLUDED