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

//TODO clean this file up, as well as the .cpp (add comments, for one)

#ifndef MOB_EVENT_INCLUDED
#define MOB_EVENT_INCLUDED

#include <vector>

#include "data_file.h"

using namespace std;

class mob;
class mob_type;
class mob_state;

typedef void (*custom_action_code)(mob* m, void* info);


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
    MOB_EVENT_ON_ENTER, //When the state is entered.
    MOB_EVENT_ON_LEAVE, //When the state is left.
    
    //Script file stuff.
    MOB_EVENT_ANIMATION_END,     //When the current animation ends.
    MOB_EVENT_ATTACK_HIT,        //When an attack hits.
    MOB_EVENT_ATTACK_MISS,       //When an attack does the whole animation without hitting.
    MOB_EVENT_BIG_DAMAGE,        //When it reaches a certain health %.
    MOB_EVENT_DAMAGE,            //When it is damaged.
    MOB_EVENT_DEATH,             //When it dies.
    MOB_EVENT_ENTER_HAZARD,      //When it enters a hazard sector.
    MOB_EVENT_LEAVE_HAZARD,      //When it leaves a hazard sector.
    MOB_EVENT_LOSE_OBJECT,       //When it loses its target object.
    MOB_EVENT_LOSE_OPPONENT,     //When it loses its opponent.
    MOB_EVENT_NEAR_OBJECT,       //When it gets near its target object.
    MOB_EVENT_NEAR_OPPONENT,     //When it gets near its opponent.
    MOB_EVENT_PIKMIN_LAND,       //When a Pikmin lands on it.
    MOB_EVENT_PIKMIN_LATCH,      //When a Pikmin latches on to it.
    MOB_EVENT_PIKMIN_TOUCH,      //When a Pikmin touches it.
    MOB_EVENT_REACH_DESTINATION, //When it reaches its destination.
    MOB_EVENT_REVIVAL,           //When it revives from being dead.
    MOB_EVENT_SEE_OBJECT,        //When it sees an object.
    MOB_EVENT_SEE_OPPONENT,      //When it sees an opponent.
    MOB_EVENT_TIMER,             //When its timer ticks.
    MOB_EVENT_WALL,              //When it touches a wall.
    
    //More internal script stuff.
    MOB_EVENT_PLUCKED,           //When it is plucked off the ground (Pikmin only).
    MOB_EVENT_GRABBED_BY_FRIEND, //When it is grabbed by a friend.
    MOB_EVENT_DISMISSED,         //When it is dismissed by its leader.
    MOB_EVENT_THROWN,            //When it is thrown.
    MOB_EVENT_LANDED,            //When it lands on the ground.
    MOB_EVENT_NEAR_TASK,         //When it is near a task (Pikmin only).
    MOB_EVENT_WHISTLED,          //When it is whistled by a leader.
    MOB_EVENT_TOUCHED_BY_LEADER, //When it is touched by a leader.
    MOB_EVENT_LEADER_IS_NEAR,    //When the leader is now near, and the mob is in the group.
    MOB_EVENT_LEADER_IS_FAR,     //When the leader is now far, and the mob is in the group.
    
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


class mob_action {
public:
    MOB_ACTION_TYPES type;
    unsigned char sub_type;
    custom_action_code code;
    bool valid;
    vector<int> vi;
    vector<float> vf;
    vector<string> vs;
    
    void run(mob* m, size_t* action_nr, void* custom_data);
    mob_action(data_node* dn, vector<mob_state*>* states, mob_type* mt);
    mob_action(MOB_ACTION_TYPES type, unsigned char sub_type = 0);
    mob_action(custom_action_code code);
};


class mob_event {
public:
    MOB_EVENT_TYPES type;
    vector<mob_action*> actions;
    
    void run(mob* m, void* custom_data = NULL);
    mob_event(data_node* d, vector<mob_action*> a);
    mob_event(const MOB_EVENT_TYPES t, vector<mob_action*> a = vector<mob_action*>());
};


class mob_state {
public:
    string name;
    unsigned short id;
    vector<mob_event*> events;
    mob_event* get_event(const unsigned short type);
    
    mob_state(const string name, vector<mob_event*> e = vector<mob_event*>());
    mob_state(const string name, const size_t id);
};


class mob_fsm {
public:
    mob* m;
    mob_state* cur_state;
    vector<size_t> pre_named_conversions; // Conversion between pre-named states and in-file states.
    
    mob_event* get_event(const unsigned short type);
    void run_event(const unsigned short type, void* custom_data = NULL);
    void set_state(const size_t new_state);
    mob_fsm(mob* m = NULL);
};


vector<mob_state*> load_script(mob_type* mt, data_node* node);

void fix_states(vector<mob_state*> &states);

#endif // ifndef MOB_EVENT_INCLUDED