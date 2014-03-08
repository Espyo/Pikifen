#ifndef MOB_EVENT_INCLUDED
#define MOB_EVENT_INCLUDED

#include <iostream>
#include <string>
#include <vector>

#include "data_file.h"

class mob;

using namespace std;

enum MOB_ACTION_TYPES {
    MOB_ACTION_UNKNOWN,
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
    MOB_ACTION_TURN,
    MOB_ACTION_WAIT,
};

enum MOB_EVENT_TYPES {
    MOB_EVENT_UNKNOWN,
    MOB_EVENT_ATTACK_HIT,
    MOB_EVENT_ATTACK_MISS,
    MOB_EVENT_BIG_DAMAGE,
    MOB_EVENT_DAMAGE,
    MOB_EVENT_DEATH,
    MOB_EVENT_ENTER_HAZARD,
    MOB_EVENT_IDLE,
    MOB_EVENT_LEAVE_HAZARD,
    MOB_EVENT_LOSE_OBJECT,
    MOB_EVENT_LOSE_PREY,
    MOB_EVENT_NEAR_OBJECT,
    MOB_EVENT_NEAR_PREY,
    MOB_EVENT_PIKMIN_LAND,
    MOB_EVENT_PIKMIN_LATCH,
    MOB_EVENT_PIKMIN_TOUCH,
    MOB_EVENT_REACH_HOME,
    MOB_EVENT_REVIVAL,
    MOB_EVENT_SEE_OBJECT,
    MOB_EVENT_SEE_PREY,
    MOB_EVENT_SPAWN,
    MOB_EVENT_TIMER,
    MOB_EVENT_WALL,
};

struct mob_event;

struct mob_action {
    unsigned char type;
    string data;
    
    mob_action(data_node* dn);
    mob_action(unsigned char t, string d);
    
    bool run(mob* m, mob_event* ev, size_t* action_nr);
};

struct mob_event {
    unsigned char type;
    vector<mob_action*> actions;
    
    mob_event(data_node* d, vector<mob_action*> a);
    mob_event(unsigned char t, vector<mob_action*> a);
    
    void run(mob* m, size_t starting_action);
};

#endif //ifndef MOB_EVENT_INCLUDED