#ifndef MOB_EVENT_INCLUDED
#define MOB_EVENT_INCLUDED

#include <iostream>
#include <string>
#include <vector>

class mob;

using namespace std;

enum MOB_ACTION_TYPES {
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
    MOB_EVENT_ATTACK_HIT,
    MOB_EVENT_ATTACK_MISS,
    MOB_EVENT_BIG_DAMAGE,
    MOB_EVENT_DAMAGE,
    MOB_EVENT_DEATH,
    MOB_EVENT_ENTER_HAZARD,
    MOB_EVENT_IDLE,
    MOB_EVENT_LEAVE_HAZARD,
    MOB_EVENT_LOSE_OBJECT,
    MOB_EVENT_LOSE_PIKMIN,
    MOB_EVENT_NEAR_OBJECT,
    MOB_EVENT_NEAR_PIKMIN,
    MOB_EVENT_PIKMIN_HIT,
    MOB_EVENT_PIKMIN_LATCH,
    MOB_EVENT_REVIVAL,
    MOB_EVENT_SEE_OBJECT,
    MOB_EVENT_SEE_PIKMIN,
    MOB_EVENT_SPAWN,
    MOB_EVENT_TIMER,
    MOB_EVENT_TOUCH_PIKMIN,
    MOB_EVENT_WALL,
};

struct mob_event;

struct mob_action {
    mob* m;
    unsigned char type;
    string data;
    
    mob_action(mob* m, unsigned char t, string d) {
        this->m = m; type = t; data = d;
    }
    
    bool run(mob_event* parent);
};

struct mob_event {
    mob* m;
    unsigned char type;
    vector<mob_action*> actions;
    size_t current_action;
    
    mob_event(mob* m, unsigned char t, vector<mob_action*> a) {
        this->m = m; type = t; actions = a;
        current_action = 0;
    }
    
    void run(bool from_the_start);
};

#endif //ifndef MOB_EVENT_INCLUDED